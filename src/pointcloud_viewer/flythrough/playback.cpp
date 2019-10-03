#include <pointcloud_viewer/flythrough/playback.hpp>

#include <glm/glm.hpp>

double Playback::current_time() const { return _current_time; }

int Playback::fixed_framerate() const { return _fixed_framerate; }

int Playback::totalNumberFramesForFixedFramerate() const {
  if (only_one_frame)
    return 1;
  else
    return int(glm::round(_animationDuration * _fixed_framerate));
}

void Playback::play_realtime() {
  stop();

  _mode = REALTIME;
  _current_time = 0.;

  started();
  request_next_frame(_current_time);
}

void Playback::play_with_fixed_framerate() {
  stop();

  _mode = FIXED_FRAMERATE;
  _current_time = 0.;

  started();
  request_next_frame(_current_time);
}

void Playback::stop() {
  if (_mode != IDLE) {
    _mode = IDLE;
    aborted();
  }
}

void Playback::previous_frame_finished(double duration) {
  duration = glm::clamp(duration, 1.e-4, 0.1);

  switch (_mode) {
    case IDLE:
      break;
    case REALTIME:
      _reqest_next_frame(current_time() + duration);
      break;
    case FIXED_FRAMERATE:
      _reqest_next_frame(current_time() + _fixed_frametime);
      break;
  }
}

void Playback::setFixed_framerate(int fixed_framerate) {
  if (_fixed_framerate == fixed_framerate) return;

  _fixed_framerate = fixed_framerate;
  _fixed_frametime = 1. / fixed_framerate;
  emit fixed_framerateChanged(_fixed_framerate);
}

Playback::Playback() {}

void Playback::_reqest_next_frame(double time) {
  if ((time > _animationDuration && _current_time <= _animationDuration) ||
      (time > 0 && only_one_frame)) {
    _current_time = time;
    _mode = IDLE;
    end_reached();
    return;
  }

  _current_time = time;

  request_next_frame(time);
}
