
#include "SceneNode.hpp"

SceneNode::SceneNode()
{
    parent = nullptr;
}

SceneNode::~SceneNode()
{}

void SceneNode::attachChild(SceneNode::Ptr child)
{
    child->parent = this;
    attachRequests.push_back(std::move(child));
}

void SceneNode::attachChildFront(SceneNode::Ptr child)
{
    child->parent = this;
    attachRequestsFront.push_back(std::move(child));
}

void SceneNode::attachToRoot(SceneNode::Ptr child)
{
    if(parent != nullptr)
    {
        parent->attachToRoot(std::move(child));
    }
    else
    {
        child->parent = this;
        attachRequests.push_back(std::move(child));
    }
}

void SceneNode::attachToRootFront(SceneNode::Ptr child)
{
    if(parent != nullptr)
    {
        parent->attachToRootFront(std::move(child));
    }
    else
    {
        child->parent = this;
        attachRequestsFront.push_back(std::move(child));
    }
}

void SceneNode::detachChild(SceneNode* node)
{
    detachRequests.push_back(node);
}

void SceneNode::clear()
{
    children.clear();
}

const glm::mat4& SceneNode::getTransform() const
{
    return transform;
}

glm::mat4 SceneNode::getWorldTransform() const
{
    if(parent != nullptr)
    {
        return transform * parent->getWorldTransform();
    }
    else
    {
        return transform;
    }
}

void SceneNode::applyTransform(const glm::mat4& transform)
{
    this->transform = transform * this->transform;
}

void SceneNode::resetTransform()
{
    transform = glm::mat4();
}

void SceneNode::update(float deltaTime)
{
    updateCurrent(deltaTime);
    updateChildren(deltaTime);

    attachChildren();
    detachChildren();
}

void SceneNode::forEach(std::function<void(SceneNode&)> function, bool includeThis, unsigned int maxDepth)
{
    forEach(function, includeThis, maxDepth, 0U);
}

void SceneNode::forEach(std::function<void(SceneNode&)> function, bool includeThis, unsigned int maxDepth, unsigned int currentDepth)
{
    if(includeThis)
    {
        function(*this);
    }

    if(maxDepth == 0U || currentDepth + 1 <= maxDepth)
    {
        for(auto node = children.begin(); node != children.end(); ++node)
        {
            (*node)->forEach(function, true, maxDepth, currentDepth + 1);
        }
    }
}

bool SceneNode::operator ==(const SceneNode& other) const
{
    return this == &other;
}

void SceneNode::detachSelf()
{
    if(parent != nullptr)
    {
        parent->detachChild(this);
    }
}

void SceneNode::draw() const
{
    drawCurrent(transform);
    drawChildren(transform);
}

void SceneNode::draw(glm::mat4 parentTransform) const
{
    glm::mat4 worldTransform = transform * parentTransform;
    drawCurrent(worldTransform);
    drawChildren(worldTransform);
}

void SceneNode::drawCurrent(glm::mat4 worldTransform) const
{}

void SceneNode::drawChildren(glm::mat4 worldTransform) const
{
    std::for_each(children.begin(), children.end(),
        [&worldTransform] (const SceneNode::Ptr& child) { child->draw(worldTransform); });
}

void SceneNode::updateCurrent(float deltaTime)
{}

void SceneNode::updateChildren(float deltaTime)
{
    std::for_each(children.begin(), children.end(),
        [&deltaTime] (SceneNode::Ptr& child) { child->update(deltaTime); });
}

void SceneNode::attachChildren()
{
    while(!attachRequests.empty())
    {
        children.push_back(std::move(attachRequests.back()));
        attachRequests.pop_back();
    }
    while(!attachRequestsFront.empty())
    {
        children.push_front(std::move(attachRequestsFront.back()));
        attachRequestsFront.pop_back();
    }
}

void SceneNode::detachChildren()
{
    while(!detachRequests.empty())
    {
        SceneNode* ptr = detachRequests.back();
        auto found = std::find_if(children.begin(), children.end(), [&ptr] (const SceneNode::Ptr& pointer) {
            return pointer.get() == ptr;
        });

        assert(found != children.end());

        children.erase(found);
        detachRequests.pop_back();
    }
}

