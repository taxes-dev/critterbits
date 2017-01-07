#pragma once
#ifndef CBANIM_HPP
#define CBANIM_HPP

#include <memory>
#include <vector>

#include "entity.hpp"
#include "scripting/scripting.hpp"

namespace Critterbits {
namespace Animation {

enum class AnimationState { Stopped, Paused, Playing, Destroyed };

typedef struct KeyFrame {
    std::string property;
    std::string value;
    int duration;

    KeyFrame(const std::string & property, const std::string & value, int duration)
        : property(property), value(value), duration(duration){};
} KeyFrame;

class Animation {
  public:
    std::string name;
    bool loop;

    Animation(const std::string & name) : name(name), loop(false){};
    virtual void Animate(std::shared_ptr<Entity>, float) = 0;
    bool IsDestroyed() { return this->state == AnimationState::Destroyed; };
    bool IsPlaying() { return this->state == AnimationState::Playing; };
    void Pause();
    void Play();
    void Stop();

  protected:
    void Destroy();
    virtual void OnDestroy(){};
    virtual void OnPause(){};
    virtual void OnPlay(){};
    virtual void OnStop(){};

  private:
    AnimationState state{AnimationState::Stopped};
};

class KeyFrameAnimation : public Animation {
  public:
    KeyFrameAnimation(const std::string & name) : Animation(name){};
    void AddKeyFrame(const KeyFrame & key_frame);
    void Animate(std::shared_ptr<Entity>, float);

  protected:
    void OnStop();

  private:
    int next_key_frame{0};
    int key_frame_count{0};
    float key_frame_delta{0.f};
    float next_key_frame_at{0.f};
    std::vector<KeyFrame> key_frames{};

    void AnimateKeyFrame(std::shared_ptr<Entity>, const KeyFrame &);
};

enum class TransformAlgorithm { Lerp, QuadEaseIn };

class TransformAnimation : public Animation {
  public:
    TransformAnimation(const TransformAlgorithm & algorithm, float duration)
        : Animation(""), algorithm(algorithm), duration(duration){};
    void Animate(std::shared_ptr<Entity>, float);
    void SetCallback(std::unique_ptr<Scripting::CB_ScriptCallback> callback);

  protected:
    float GetTransformedValue(float, float, float);
    CB_Point GetTransformedValue(CB_Point, CB_Point, float);
    virtual void AnimateValues(std::shared_ptr<Entity>, float) = 0;
    virtual void OnStop() { this->Destroy(); };

  private:
    TransformAlgorithm algorithm;
    float duration;
    float elapsed{0.f};
    std::unique_ptr<Scripting::CB_ScriptCallback> callback;
};

class TranslateAnimation : public TransformAnimation {
  public:
    TranslateAnimation(const TransformAlgorithm & algorithm, float duration, CB_Point start, CB_Point end)
        : TransformAnimation(algorithm, duration), start(start), end(end){};

  protected:
    void AnimateValues(std::shared_ptr<Entity>, float);

  private:
    CB_Point start, end;
};
}
}
#endif