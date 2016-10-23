
#ifndef SCENE_NODE_HPP
#define SCENE_NODE_HPP

#include <memory>
#include <algorithm>
#include <cassert>
#include <functional>
#include <vector>
#include <list>

#include <glm/glm.hpp>

#ifndef NDEBUG
  #include <iostream>
#endif

class SceneNode
{
public:
    typedef std::unique_ptr<SceneNode> Ptr;

    SceneNode();
    virtual ~SceneNode();

    void attachChild(Ptr child);
    void attachChildFront(Ptr child);
    void attachToRoot(Ptr child);
    void attachToRootFront(Ptr child);
    void detachChild(SceneNode* node);
    void clear();

    const glm::mat4& getTransform() const;
    void applyTransform(const glm::mat4& transform);
    void resetTransform();

    void update(float deltaTime);

    void forEach(std::function<void(SceneNode&)> function, bool includeThis = false, unsigned int maxDepth = 0U);

private:
    void forEach(std::function<void(SceneNode&)> function, bool includeThis, unsigned int maxDepth, unsigned int currentDepth);

public:
    bool operator ==(const SceneNode& other) const;

protected:
    void detachSelf();

public:
    void draw() const;

private:
    void draw(glm::mat4 parentTransform) const;
    virtual void drawCurrent(glm::mat4 worldTransform) const;
    void drawChildren(glm::mat4 worldTransform) const;

    virtual void updateCurrent(float deltaTime);
    void updateChildren(float deltaTime);

    void attachChildren();
    void detachChildren();

    glm::mat4 getWorldTransform() const;

    std::list<Ptr>    children;
    std::vector<SceneNode::Ptr> attachRequests;
    std::vector<SceneNode::Ptr> attachRequestsFront;
    std::vector<SceneNode*> detachRequests;
    SceneNode*          parent;

    glm::mat4 transform;

};

#endif

