#ifndef POINTCLOUDVIEWER_WORKERS_IMPORTPOINTCLOUD_HPP_
#define POINTCLOUDVIEWER_WORKERS_IMPORTPOINTCLOUD_HPP_

#include <QObject>
#include <pointcloud/pointcloud.hpp>

/**
The function responsible for import point clouds.
*/
QSharedPointer<PointCloud> import_point_cloud(QWidget* parent, QString file);

#endif  // POINTCLOUDVIEWER_WORKERS_IMPORTPOINTCLOUD_HPP_
