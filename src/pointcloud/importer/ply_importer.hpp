#ifndef POINTCLOUD_WORKERS_IMPORTPOINTCLOUD_HPP_
#define POINTCLOUD_WORKERS_IMPORTPOINTCLOUD_HPP_

#include <pointcloud/importer/abstract_importer.hpp>

/**
Implementation for loading ply files
*/

class PlyImporter final : public AbstractPointCloudImporter
{
  Q_OBJECT

public:
  PlyImporter(std::istream& input_stream, int64_t total_num_bytes);

protected:
  bool import_implementation() override;
};

#endif // POINTCLOUD_WORKERS_IMPORTPOINTCLOUD_HPP_
