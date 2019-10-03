#include <fstream>
#include <pointcloud/importer/pcvd_importer.hpp>
#include <pointcloud/pcvd_file_format.hpp>

PcvdImporter::PcvdImporter(const std::string& input_file)
    : AbstractPointCloudImporter(input_file) {}

bool PcvdImporter::import_implementation() {
  std::streamsize read_bytes;
  std::ifstream stream(
      input_file,
      std::ios_base::in | std::ios_base::binary);  // a binary stream

  auto read = [&stream](void* target,
                        std::streamsize num_bytes) -> std::streamsize {
    std::streamsize begin = stream.tellg();

    stream.read(reinterpret_cast<char*>(target), num_bytes);

    return stream.tellg() - begin;
  };

  pcvd_format::header_t header;
  read_bytes = read(&header, sizeof(pcvd_format::header_t));
  if (read_bytes == 0) throw QString("Can't load empty file");

  if (read_bytes < 4) throw QString("Can't load corrupt file");

  if (header.magic_number != pcvd_format::header_t::expected_macic_number())
    throw QString("Wrong file format");

  if (read_bytes != sizeof(pcvd_format::header_t))
    throw QString("Can't load corrupt file");

  if (header.downwards_compatibility_version_number > 1)
    throw QString("Incompatible file format version");

  if (header.number_points == 0) throw QString("Need at least one point");
  if (header.number_fields == 0) throw QString("Need at least one field");
  if (header.point_data_stride == 0)
    throw QString("corrupt header (point_data_stride)");
  if (header.field_names_total_size == 0)
    throw QString("corrupt header (field_names_total_size)");

  if (header.file_version_number == 0 && (header.flags & 0xfffc) != 0)
    throw QString("corrupt header (invalid flags)");
  if (header.file_version_number == 1 && (header.flags & 0xfff8) != 0)
    throw QString("corrupt header (invalid flags)");
  if (header.file_version_number < 1 && header.shader_data_size != 0)
    throw QString("corrupt header (invalid padding)");
  if (header.reserved != 0) throw QString("corrupt header (invalid padding)");
  if (glm::any(glm::isnan(header.aabb.min_point)))
    throw QString("corrupt header (invalid aabb)");
  if (glm::any(glm::isnan(header.aabb.max_point)))
    throw QString("corrupt header (invalid aabb)");
  if (glm::any(glm::isinf(header.aabb.min_point)))
    throw QString("corrupt header (invalid aabb)");
  if (glm::any(glm::isinf(header.aabb.max_point)))
    throw QString("corrupt header (invalid aabb)");

  const bool load_kd_tree = header.flags & 0b1;
  const bool load_vertex = header.flags & 0b10;
  const bool load_shader = header.flags & 0b100;

  std::streamsize header_size = sizeof(pcvd_format::header_t);
  std::streamsize field_headers_size =
      sizeof(pcvd_format::field_description_t) * header.number_fields;
  std::streamsize field_names_size = header.field_names_total_size;
  std::streamsize vertex_data_size =
      std::streamsize(header.number_points * sizeof(PointCloud::vertex_t));
  std::streamsize point_data_size =
      std::streamsize(header.number_points * header.point_data_stride);
  std::streamsize kd_tree_size =
      load_kd_tree ? std::streamsize(header.number_points * sizeof(size_t)) : 0;
  std::streamsize shader_size =
      load_shader ? std::streamsize(sizeof(pcvd_format::shader_description_t) +
                                    header.shader_data_size)
                  : 0;
  total_progress = header_size + field_headers_size + field_names_size +
                   vertex_data_size + point_data_size + kd_tree_size +
                   shader_size;

  handle_loaded_chunk(current_progress += header_size);

  QVector<pcvd_format::field_description_t> field_descriptions;
  std::string joined_field_names;
  field_descriptions.resize(header.number_fields);
  joined_field_names.resize(header.field_names_total_size);
  read_bytes = read(field_descriptions.data(), field_headers_size);
  if (read_bytes != field_headers_size) throw QString("Incomplete file!");
  read_bytes = read(&joined_field_names.at(0), field_names_size);
  if (read_bytes != field_names_size) throw QString("Incomplete file!");

  QVector<QString> field_names;
  QVector<data_type::base_type_t> field_types;
  QVector<size_t> field_data_offset;
  field_names.reserve(header.number_fields);
  field_types.reserve(header.number_fields);
  field_data_offset.reserve(header.number_fields);
  uint16_t fields_total_stride = 0;
  uint16_t fields_total_name_length = 0;
  for (int i = 0; i < header.number_fields; ++i) {
    const uint16_t begin_name = fields_total_name_length;
    const uint16_t end_name = begin_name + field_descriptions[i].name_length;

    if (begin_name == end_name) throw QString("Corrupt property! (empty name)");
    if (end_name > joined_field_names.length())
      throw QString("Corrupt property! (buffer overflow)");

    const data_type::base_type_t base_type = field_descriptions[i].type;
    const QString name = QString::fromStdString(
        std::string(joined_field_names.data() + begin_name,
                    joined_field_names.data() + end_name));

    if (!data_type::is_valid(base_type))
      throw QString("Corrupt property! (invalid type)");

    field_names << name;
    field_types << base_type;
    field_data_offset << fields_total_stride;

    fields_total_stride += data_type::size_of_type(base_type);
    fields_total_name_length += field_descriptions[i].name_length;
  }

  if (fields_total_stride != header.point_data_stride)
    throw QString("Corrupt header! (point data stride mismatch)");
  if (fields_total_name_length != header.field_names_total_size)
    throw QString("Corrupt header! (field names length mismatch)");

  pointcloud.aabb = header.aabb;
  pointcloud.user_data_stride = header.point_data_stride;
  pointcloud.user_data_names = field_names;
  pointcloud.user_data_offset = field_data_offset;
  pointcloud.user_data_types = field_types;
  pointcloud.resize(header.number_points);

  handle_loaded_chunk(current_progress +=
                      field_headers_size + field_names_size);

  if (load_vertex) {
    read_bytes = read(pointcloud.coordinate_color.data(), vertex_data_size);
    if (read_bytes != vertex_data_size) throw QString("Incomplete file!");
    handle_loaded_chunk(current_progress += vertex_data_size);
  }

  read_bytes = read(pointcloud.user_data.data(), point_data_size);
  if (read_bytes != point_data_size) throw QString("Incomplete file!");
  handle_loaded_chunk(current_progress += point_data_size);

  if (!load_vertex) {
    uint8_t* coordinates = pointcloud.coordinate_color.data();

    size_t ui_update = 0;
    for (size_t i = 0; i < header.number_points; ++i) {
      PointCloud::vertex_t vertex;

      vertex.coordinate.x = std::numeric_limits<float>::quiet_NaN();
      vertex.coordinate.y = std::numeric_limits<float>::quiet_NaN();
      vertex.coordinate.z = std::numeric_limits<float>::quiet_NaN();
      vertex.color.r = 255;
      vertex.color.g = 0;
      vertex.color.b = 255;

      write_value_to_buffer<PointCloud::vertex_t>(coordinates, vertex);

      coordinates += PointCloud::stride;
    }
    handle_loaded_chunk(current_progress += ui_update * PointCloud::stride);
  }

  if (load_kd_tree) {
    read_bytes = read(pointcloud.kdtree_index.alloc_for_loading(
                          header.number_points, header.aabb),
                      kd_tree_size);
    if (read_bytes != kd_tree_size) throw QString("Incomplete file!");
    handle_loaded_chunk(current_progress += kd_tree_size);
  }

  if (load_shader) {
    QByteArray text_data;
    pcvd_format::shader_description_t shader_description;

    read_bytes = read(&shader_description, sizeof(shader_description));
    if (read_bytes != sizeof(pcvd_format::shader_description_t))
      throw QString("Incomplete file!");
    handle_loaded_chunk(current_progress +=
                        sizeof(pcvd_format::shader_description_t));

    text_data.resize(shader_description.used_properties_length);
    read(text_data.data(), shader_description.used_properties_length);
    pointcloud.shader.used_properties =
        QString::fromUtf8(text_data).split('\n').toSet();

    text_data.resize(shader_description.coordinate_expression_length);
    read(text_data.data(), shader_description.coordinate_expression_length);
    pointcloud.shader.coordinate_expression = QString::fromUtf8(text_data);

    text_data.resize(shader_description.color_expression_length);
    read(text_data.data(), shader_description.color_expression_length);
    pointcloud.shader.color_expression = QString::fromUtf8(text_data);

    text_data.resize(shader_description.node_data_length);
    read(text_data.data(), shader_description.node_data_length);
    pointcloud.shader.node_data = QString::fromUtf8(text_data);
  }

  return true;
}
