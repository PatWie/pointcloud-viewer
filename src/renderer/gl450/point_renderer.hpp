#ifndef RENDERSYSTEM_GL450_POINT_RENDERER_HPP_
#define RENDERSYSTEM_GL450_POINT_RENDERER_HPP_

#include <renderer/gl450/declarations.hpp>

#include <glhelper/buffer.hpp>
#include <glhelper/shaderobject.hpp>
#include <glhelper/vertexarrayobject.hpp>

namespace renderer {
namespace gl450 {

/**
Renderer responsible for rendering a single point-cloud.
Multiple of those can be used for having multiple layers of point clouds.
*/
class PointRenderer final {
 public:
  PointRenderer();
  ~PointRenderer();

  PointRenderer(PointRenderer&& point_renderer);
  PointRenderer& operator=(PointRenderer&& point_renderer);

  void clear_buffer();
  void load_points(const uint8_t* point_data, GLsizei num_points);
  void load_test(GLsizei num_vertices = 512);

  void render_points();

 private:
  gl::ShaderObject shader_object;
  gl::Buffer vertex_position_buffer;
  gl::VertexArrayObject vertex_array_object;
  GLsizei num_vertices = 0;
};

}  // namespace gl450
}  // namespace renderer

#endif  // RENDERSYSTEM_GL450_POINT_RENDERER_HPP_
