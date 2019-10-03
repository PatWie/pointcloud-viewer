#include <core_library/image.hpp>
#include <pointcloud_viewer/flythrough/flythrough.hpp>
#include <pointcloud_viewer/viewport.hpp>
#include <pointcloud_viewer/workers/offline_renderer.hpp>

#include <QDebug>
#include <QMessageBox>
#include <QTimer>

OfflineRenderer::OfflineRenderer(Viewport* viewport,
                                 const Flythrough& flythrough,
                                 const RenderSettings& renderSettings)
    : viewport(*viewport),
      flythrough(flythrough.copy()),
      renderSettings(renderSettings),
      result_rgba(renderSettings.resolution.width(),
                  renderSettings.resolution.height(), gl::TextureFormat::RGB8),
      result_depth(renderSettings.resolution.width(),
                   renderSettings.resolution.height(),
                   gl::TextureFormat::DEPTH_COMPONENT32F),
      framebuffer(gl::FramebufferObject::Attachment(&result_rgba),
                  gl::FramebufferObject::Attachment(&result_depth)) {
  this->flythrough->playback.setFixed_framerate(renderSettings.framerate);

  connect(this->flythrough.data(), &Flythrough::set_new_camera_frame, this,
          &OfflineRenderer::render_next_frame, Qt::DirectConnection);
  connect(&this->flythrough->playback, &Playback::aborted, this,
          &OfflineRenderer::abort, Qt::DirectConnection);
  connect(&this->flythrough->playback, &Playback::end_reached, this,
          &OfflineRenderer::finished, Qt::DirectConnection);
  connect(this, &OfflineRenderer::rendered_frame, &this->flythrough->playback,
          &Playback::previous_frame_finished, Qt::QueuedConnection);

  if (renderSettings.export_images)
    connect(this, &OfflineRenderer::rendered_frame, this,
            &OfflineRenderer::save_image);

  viewport->enable_preview = false;
}

OfflineRenderer::~OfflineRenderer() { viewport.enable_preview = true; }

bool OfflineRenderer::was_aborted() const { return _aborted; }

void OfflineRenderer::start() {
  _aborted = false;
  frame_index = 0;
  flythrough->playback.play_with_fixed_framerate();
}

void OfflineRenderer::abort() {
  if (_aborted == true) return;

  _aborted = true;
  on_aborted();
}

void OfflineRenderer::render_next_frame(frame_t camera_frame) {
  if (_aborted) return;

  const int width = renderSettings.resolution.width();
  const int height = renderSettings.resolution.height();

  QImage frame_content(renderSettings.resolution.width(),
                       renderSettings.resolution.height(),
                       QImage::Format_RGB888);

  viewport.makeCurrent();

  const GLuint fbo = framebuffer.GetInternHandle();
  GL_CALL(glBindBuffer, GL_PIXEL_PACK_BUFFER, 0);
  GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, fbo);
  GL_CALL(glViewport, 0, 0, width, height);

  viewport.render_points(camera_frame, float(width) / float(height), []() {});

  GL_CALL(glNamedFramebufferReadBuffer, fbo, GL_COLOR_ATTACHMENT0);
  GL_CALL(glReadPixels, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
          frame_content.bits());
  GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, 0);
  viewport.doneCurrent();

  flip_image(frame_content);

  ++frame_index;
  rendered_frame(frame_index - 1, frame_content);
}

void OfflineRenderer::save_image(int frame_index, const QImage& image) {
  QDir target_image_dir(renderSettings.target_images_directory);

  QString filepath = target_image_dir.absoluteFilePath(
      QString("frame_%0%1")
          .arg(frame_index + renderSettings.first_index,
               5 /* how many digits to expect, for example 2 leads to 04*/,
               10 /* base */, QChar('0'))
          .arg(renderSettings.image_format));

  qDebug() << filepath;
  image.save(filepath);
}
