#ifndef POINTCLOUDVIEWER_VIEWPORT_HPP_
#define POINTCLOUDVIEWER_VIEWPORT_HPP_

#include <pointcloud/pointcloud.hpp>
#include <pointcloud/pointcloud.hpp>
#include <pointcloud_viewer/camera.hpp>
#include <pointcloud_viewer/declarations.hpp>
#include <pointcloud_viewer/navigation.hpp>
#include <renderer/gl450/declarations.hpp>

#include <QOpenGLWidget>
#include <functional>
#include <unordered_map>

/*
The viewport is owning the opengl context and delegating the point rendering to
the renderer.
*/

class Viewport final : public QOpenGLWidget {
  Q_OBJECT
  Q_PROPERTY(int backgroundColor READ backgroundColor WRITE setBackgroundColor
                 NOTIFY backgroundColorChanged)
  Q_PROPERTY(
      int pointSize READ pointSize WRITE setPointSize NOTIFY pointSizeChanged)
 public:
  Navigation navigation;
  bool enable_preview = true;

  Viewport();
  ~Viewport() override;

  aabb_t aabb() const;

  void set_camera_frame(const frame_t& frame);

  void unload_all_point_clouds();
  void load_point_cloud(QSharedPointer<PointCloud> point_cloud);

  // Only MainWindow::apply_point_shader is allowed to call this function
  bool reapply_point_shader(bool reapply_point_shader);

  void render_points(frame_t camera_frame, float aspect,
                     std::function<void()> additional_rendering) const;

  int backgroundColor() const;
  int pointSize() const;

  Visualization& visualization() { return *_visualization; }

 public slots:
  void setBackgroundColor(int backgroundColor);
  void setPointSize(int pointSize);

 signals:
  void frame_rendered(double duration);

  void backgroundColorChanged(int backgroundColor);
  void pointSizeChanged(int pointSize);

  void openGlContextCreated();

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void paintEvent(QPaintEvent* event) override;

  void wheelEvent(QWheelEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

 private:
  typedef renderer::gl450::PointRenderer PointRenderer;
  typedef renderer::gl450::GlobalUniform GlobalUniform;

  PointRenderer* point_renderer = nullptr;
  GlobalUniform* global_uniform = nullptr;

  Visualization* _visualization;

  aabb_t _aabb = aabb_t::invalid();
  QSharedPointer<PointCloud> point_cloud;
  size_t next_handle = 0;
  int m_backgroundColor = 0;
  int m_pointSize = 1;
};

#endif  // POINTCLOUDVIEWER_VIEWPORT_HPP_
