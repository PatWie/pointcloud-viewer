#ifndef POINTCLOUDVIEWER_FLYTHROUGH_INTERPOLATION_HPP_
#define POINTCLOUDVIEWER_FLYTHROUGH_INTERPOLATION_HPP_

#include <pointcloud_viewer/flythrough/keypoint.hpp>

#include <QVector>

/**
Interface for different keypoint interpolation techniques.
*/
class Interpolation {
 public:
  Q_DISABLE_COPY(Interpolation)

  const QVector<keypoint_t>& keypoints;

  Interpolation(const QVector<keypoint_t>* keypoints);
  virtual ~Interpolation();

  virtual double path_length() const = 0;
  virtual frame_t frame_for_overcome_distance(double distance) const = 0;
};

class LinearInterpolation : public Interpolation {
 public:
  LinearInterpolation(const QVector<keypoint_t>* keypoints,
                      bool smoothstep = false);

  const bool smoothstep;

  double path_length() const override;
  frame_t frame_for_overcome_distance(double distance) const override;
};
#endif  // POINTCLOUDVIEWER_FLYTHROUGH_INTERPOLATION_HPP_
