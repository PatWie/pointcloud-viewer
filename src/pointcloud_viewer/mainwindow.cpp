#include <core_library/print.hpp>
#include <pointcloud_viewer/mainwindow.hpp>

#include <QMessageBox>

MainWindow::MainWindow()
    : kdTreeInspector(this),
      pointCloudInspector(&viewport),
      pointShaderEditor(this) {
  setWindowTitle("Pointcloud Viewer");

  setCentralWidget(&viewport);

  connect(&flythrough, &Flythrough::set_new_camera_frame, &viewport,
          &Viewport::set_camera_frame);
  connect(&viewport, &Viewport::frame_rendered, &flythrough.playback,
          &Playback::previous_frame_finished);
  connect(&viewport, &Viewport::openGlContextCreated, this,
          &MainWindow::handleApplicationArguments);

  connect(&viewport.navigation, &Navigation::picked_point, &pointCloudInspector,
          &PointCloudInspector::pick_point);
  connect(&viewport.navigation, &Navigation::annotated_point, &pointCloudInspector,
          &PointCloudInspector::annotate_point);
  connect(&viewport, &Viewport::pointSizeChanged, &pointCloudInspector,
          &PointCloudInspector::setPickRadius);

  connect(this, &MainWindow::pointcloud_unloaded, [this]() {
    pointcloud.clear();
    loadedShader = PointCloud::Shader();
    viewport.unload_all_point_clouds();
    pointShaderEditor.unload_all_point_clouds();
    kdTreeInspector.unload_all_point_clouds();
    pointCloudInspector.unload_all_point_clouds();
  });
  connect(this, &MainWindow::pointcloud_imported,
          [this](QSharedPointer<PointCloud> p) {
            pointcloud = p;
            viewport.load_point_cloud(p);
            pointShaderEditor.load_point_cloud(p);
            kdTreeInspector.handle_new_point_cloud(p);
            pointCloudInspector.handle_new_point_cloud(p);
            viewport.navigation.handle_new_point_cloud();
            if (glm::any(glm::isnan(p->vertex(0).coordinate)))
              this->apply_point_shader(p->shader, true, true);
            loadedShader = p->shader;
          });

  initMenuBar();
  initDocks();
}

MainWindow::~MainWindow() {}

bool MainWindow::apply_point_shader(PointCloud::Shader new_shader,
                                    bool coordinates_changed,
                                    bool colors_changed) {
  if (this->pointcloud == nullptr) return false;

  PointCloud::Shader autogenerated_shader =
      pointShaderEditor.autogenerate(pointcloud.data());
  PointCloud::Shader old_shader = this->pointcloud->shader;
  this->pointcloud->shader = new_shader;

  const bool needs_being_rebuilt_for_the_first_time =
      glm::any(glm::isnan(this->pointcloud->vertex(0).coordinate));
  const bool had_some_changes = coordinates_changed || colors_changed;

  if (!needs_being_rebuilt_for_the_first_time && !had_some_changes)
    return false;

  const QSet<QString> properties_provided_by_pointcloud =
      this->pointcloud->user_data_names.toList().toSet();
  const QSet<QString> properties_requested_by_shader =
      new_shader.used_properties;
  QStringList properties_requested_by_shader_but_not_provided_by_pointcloud =
      (properties_requested_by_shader - properties_provided_by_pointcloud)
          .toList();
  properties_requested_by_shader_but_not_provided_by_pointcloud.sort();

  const bool shader_contains_unexpected_values =
      properties_requested_by_shader_but_not_provided_by_pointcloud.isEmpty() ==
      false;

  if (shader_contains_unexpected_values) {
    QMessageBox::warning(
        this, "Incompatible Shader",
        "Can't apply the shader to the pointcloud, as the pointcloud doesn't "
        "provide the following properties requested by the shader:\n* " +
            properties_requested_by_shader_but_not_provided_by_pointcloud.join(
                "\n* "));
    this->pointcloud->shader = autogenerated_shader;
  } else if (old_shader.color_expression.isEmpty() &&
             old_shader.coordinate_expression.isEmpty()) {
    this->pointcloud->shader = autogenerated_shader;
  }

  if (this->pointcloud->shader.coordinate_expression.isEmpty())
    this->pointcloud->shader.coordinate_expression =
        autogenerated_shader.coordinate_expression;
  if (this->pointcloud->shader.color_expression.isEmpty())
    this->pointcloud->shader.color_expression =
        autogenerated_shader.color_expression;

  if (!viewport.reapply_point_shader(coordinates_changed)) return false;

  // update the selected point
  pointCloudInspector.update();

  return true;
}
