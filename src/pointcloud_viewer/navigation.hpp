﻿#ifndef POINTCLOUDVIEWER_NAVIGATION_HPP_
#define POINTCLOUDVIEWER_NAVIGATION_HPP_

#include <pointcloud_viewer/camera.hpp>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>

class Viewport;
class UsabilityScheme;

/*
Handles input events for implementing navigation
*/
class Navigation final : public QObject {
  Q_OBJECT
 public:
  class Controller;

  Camera camera;
  glm::vec3 turntable_origin = glm::vec3(0);

  Navigation(Viewport* viewport);
  ~Navigation();

  UsabilityScheme& usabilityScheme();

  void unsetSelectedPoint();
  void setSelectedPoint(glm::vec3 selectedPoint);

 public slots:
  void startFpsNavigation();
  void stopFpsNavigation(bool keepNewFrame = true);

  void resetCameraLocation();
  void resetMovementSpeed();

  void updateFrameRenderDuration(double duration);

  void wheelEvent(QWheelEvent* event);
  void mouseMoveEvent(QMouseEvent* event);
  void mousePressEvent(QMouseEvent* event);
  void mouseReleaseEvent(QMouseEvent* event);
  void mouseDoubleClickEvent(QMouseEvent* event);
  void keyPressEvent(QKeyEvent* event);
  void keyReleaseEvent(QKeyEvent* event);
  void focusOutEvent(QFocusEvent* event);

  glm::ivec2 mouse_sensitivity_value_range() const;
  int mouse_sensitivity_value() const;
  void set_mouse_sensitivity_value(int value);

  void handle_new_point_cloud();

 protected:
  void timerEvent(QTimerEvent* timerEvent);

 signals:
  void mouse_sensitivity_value_changed(int value);

  void picked_point(glm::ivec2 point);
  void annotated_point(glm::ivec2 point, int label);

 private:
  enum distance_t {
    CLOSE,
    FAR,
    VERY_FAR,
  };

  Viewport* const viewport;

  Controller* const _controller;
  UsabilityScheme* const _usability_scheme;

  bool fps_mode = false;

  glm::vec3 trackball_center = glm::vec3(0);
  float trackball_radius = 1.f;
  void update_trackball_radius();
  glm::vec3 trackball_position_right_infront_of_camera() const;

  static glm::ivec2 invalid_last_mouse_pos() {
    return glm::ivec2(std::numeric_limits<int>::min());
  }
  glm::ivec2 last_mouse_pos = invalid_last_mouse_pos();

  glm::ivec2 viewport_center() const;
  distance_t distance(glm::ivec2 difference, glm::ivec2 radius) const;

  void tilt_camera(double factor);
  void reset_camera_tilt();

  void incr_base_movement_speed(int incr);
  float base_movement_speed() const;
  float mouse_sensitivity() const;

  float _last_frame_duration = 1 / 60.f;

  int _base_movement_speed = 0;
  int _mouse_sensitivity_value = 0;
  frame_t fps_start_frame;
  int fps_timer = 0;
  int num_frames_in_fps_mode = 0;

  bool _has_selected_point = false;
  glm::vec3 _selected_point;

  glm::vec3 find_best_turntable_origin();
  glm::vec3 _turntable_origin_relative_to_camera;

  void setTurntableOrigin(glm::vec3 origin);

  void navigate_fps();
  void fps_rotation(glm::vec2 mouse_force);

  void set_mouse_pos(glm::ivec2 mouse_pos);

  void pick_point(const glm::ivec2 screenspace_pixel);
  void annotate_point(const glm::ivec2 screenspace_pixel, int label);
};

class Navigation::Controller final {
 public:
  glm::vec3 key_direction = glm::vec3(0.f);
  int key_speed = 0;
  glm::vec3 key_force = glm::vec3(0.f);

  Camera& camera;

  Controller(const Controller&) = delete;
  Controller& operator=(const Controller&) = delete;

  Controller(Controller&&) = delete;
  Controller& operator=(Controller&&) = delete;

  void pick_point(const glm::ivec2 screenspace_pixel);
  void annotate_point(const glm::ivec2 screenspace_pixel, int label);

  void incr_base_movement_speed(int incr);
  void tilt_camera(double factor);
  void reset_camera_tilt();

  void startFpsNavigation();
  void stopFpsNavigation(bool keepNewFrame = true);

  void show_trackball();
  void begin_trackball_action();
  void trackball_rotate(glm::vec2 mouse_force, glm::ivec2 screenspace_pixel);
  void trackball_shift(glm::vec2 mouse_force);
  void trackball_zoom(float mouse_force_y);
  void end_trackball_action();
  void hide_trackball();
  void zoom_trackball_to_current_point();

  void show_grid();
  void hide_grid();
  void begin_turntable_action();
  void end_turntable_action();
  void turntable_rotate(glm::vec2 mouse_force);
  void turntable_shift(glm::vec2 mouse_force);
  void turntable_zoom(float mouse_force_y);
  void zoom_turntable_to_current_point();

  void incr_point_render_size(int incr);

  glm::vec3 forward_vector() const;
  glm::vec3 up_vector() const;
  glm::vec3 right_vector() const;

  void update_key_force();

 private:
  friend class Navigation;

  Navigation& navigation;

  Controller(Navigation& navigation);

  void _rotate(glm::vec3 rotation_center, glm::vec2 mouse_force,
               glm::vec3 x_rotation_axis, glm::vec3 y_rotation_axis);
  glm::vec3 _shift(glm::vec2 mouse_force);
  void _zoom(glm::vec3 origin, float mouse_force_y);
};

#endif  // POINTCLOUDVIEWER_NAVIGATION_HPP_
