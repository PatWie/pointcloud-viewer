#include <core_library/print.hpp>
#include <glhelper/gl.hpp>
#include <renderer/gl450/locate_shaders.hpp>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

namespace renderer {
namespace gl450 {

void locate_shaders() {
  QVector<QDir> candidate_dirs;

  QDir binaryDirectory = QCoreApplication::applicationDirPath();

  candidate_dirs << binaryDirectory.absoluteFilePath("shader");
  candidate_dirs << QFileInfo(__FILE__).dir().absoluteFilePath("shader");

  for (QDir candidate : candidate_dirs) {
    if (candidate.exists() && candidate.exists("uniforms/global.vs.glsl")) {
      gl::Details::ShaderIncludeDirManager::addIncludeDirs(candidate);
      return;
    }
  }

  print_error(
      "Couldn't locate the shader files."
      "Please copy the shaders directory from ",
      QFileInfo(__FILE__).dir().absolutePath().toStdString(), " to ",
      binaryDirectory.absolutePath().toStdString());
  std::abort();
}

}  // namespace gl450
}  // namespace renderer
