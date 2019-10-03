#include <QtGlobal>
#include <geometry/aabb.hpp>
#include <glm/gtx/io.hpp>

aabb_t aabb_t::fromVertices(const glm::vec3* vertices, int numVertices) {
  return fromVertices(vertices, numVertices, sizeof(glm::vec3));
}

aabb_t aabb_t::invalid() {
  aabb_t aabb;
  aabb.min_point = glm::vec3(INFINITY);
  aabb.max_point = glm::vec3(-INFINITY);

  return aabb;
}

bool aabb_t::contains(glm::vec3 point, float epsilon) const {
  return glm::all(glm::lessThanEqual(this->min_point - epsilon, point)) &&
         glm::all(glm::lessThanEqual(point, this->max_point + epsilon));
}

std::pair<aabb_t, aabb_t> aabb_t::split(int split_dimension,
                                        glm::vec3 split_point) const {
  aabb_t left = *this;
  aabb_t right = *this;

  left.max_point[split_dimension] =
      glm::clamp(split_point[split_dimension], min_point[split_dimension],
                 max_point[split_dimension]);
  right.min_point[split_dimension] =
      glm::clamp(split_point[split_dimension], min_point[split_dimension],
                 max_point[split_dimension]);

  return std::make_pair(left, right);
}

glm::vec3 aabb_t::toUnitSpace(const glm::vec3& v) const {
  return (v - this->min_point) / (this->max_point - this->min_point);
}

glm::vec3 aabb_t::center_point() const {
  return glm::mix(min_point, max_point, 0.5f);
}

glm::vec3 aabb_t::size() const { return max_point - min_point; }

void aabb_t::operator|=(const aabb_t& other) {
  Q_ASSERT(other.is_valid());
  *this |= other.max_point;
  *this |= other.min_point;
}

void aabb_t::operator|=(const glm::vec3& other) {
  this->min_point = glm::min(other, this->min_point);
  this->max_point = glm::max(other, this->max_point);
}

aabb_t aabb_t::fromVertices(const glm::vec3* vertices, int num_vertices,
                            size_t stride) {
  aabb_t aabb = aabb_t::invalid();
  size_t address = size_t(vertices);

  for (int i = 0; i < num_vertices; ++i) {
    const glm::vec3& v = *reinterpret_cast<const glm::vec3*>(address);

    aabb |= v;

    address += stride;
  }

  return aabb;
}

aabb_t aabb_t::aabbOfTransformedBoundingBox(const frame_t& coord_frame) const {
  glm::vec3 p[2] = {this->min_point, this->max_point};

  aabb_t aabb = aabb_t::invalid();
  for (int i = 0; i < 8; ++i) {
    glm::ivec3 p_index((i & 4) > 0, (i & 2) > 0, (i & 1) > 0);

    glm::vec3 v = coord_frame.transform_point(
        glm::vec3(p[p_index.x].x, p[p_index.y].y, p[p_index.z].z));

    aabb |= v;
  }

  return aabb;
}

aabb_t aabb_t::ensureValid() const {
  aabb_t aabb = *this;

  if (!is_valid()) {
    aabb.min_point = glm::vec3(-1);
    aabb.max_point = glm::vec3(1);
  }

  return aabb;
}

std::ostream& operator<<(std::ostream& stream, const aabb_t& aabb) {
  return stream << "aabb_t{min:" << aabb.min_point << ", max:" << aabb.max_point
                << "}";
}
