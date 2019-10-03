#ifndef POINTCLOUD_WORKERS_IMPORTER_PLY_HPP_
#define POINTCLOUD_WORKERS_IMPORTER_PLY_HPP_

#include <pointcloud/buffer.hpp>
#include <pointcloud/importer/abstract_importer.hpp>

#include <pcl/io/ply/ply_parser.h>

#include <QVector>

/**
Implementation for loading ply files
*/
class PlyImporter final : public AbstractPointCloudImporter {
 public:
  PlyImporter(const std::string& input_file);

 protected:
  bool import_implementation() override;

 private:
  int64_t current_progress;

  size_t vertex_data_stride;
  QVector<QString> property_names;
  QVector<size_t> property_offsets;
  QVector<data_type::base_type_t> property_types;

  template <typename value_type>
  typename pcl::io::ply::ply_parser::scalar_property_definition_callback_type<
      value_type>::type
  property_callback_handler(PointCloud::vertex_t** new_vertex_x,
                            PointCloud::vertex_t** new_vertex_y,
                            PointCloud::vertex_t** new_vertex_z,
                            PointCloud::vertex_t** new_vertex_r,
                            PointCloud::vertex_t** new_vertex_g,
                            PointCloud::vertex_t** new_vertex_b,
                            uint8_t** all_data);
};

#endif  // POINTCLOUD_WORKERS_IMPORTER_PLY_HPP_
