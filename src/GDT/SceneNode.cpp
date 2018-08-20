
#include "SceneNode.hpp"

GDT::SceneNode::SceneNode()
{
    parent = nullptr;
}

GDT::SceneNode::~SceneNode()
{}

void GDT::SceneNode::attachChild(GDT::SceneNode::Ptr child)
{
    child->parent = this;
    attachRequests.push_back(std::move(child));
}

void GDT::SceneNode::attachChildFront(GDT::SceneNode::Ptr child)
{
    child->parent = this;
    attachRequestsFront.push_back(std::move(child));
}

void GDT::SceneNode::attachToRoot(GDT::SceneNode::Ptr child)
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

void GDT::SceneNode::attachToRootFront(GDT::SceneNode::Ptr child)
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

void GDT::SceneNode::detachChild(SceneNode* node)
{
    detachRequests.push_back(node);
}

void GDT::SceneNode::clear()
{
    children.clear();
}

const glm::mat4& GDT::SceneNode::getTransform() const
{
    return transform;
}

glm::mat4 GDT::SceneNode::getWorldTransform() const
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

void GDT::SceneNode::applyTransform(const glm::mat4& transform)
{
    this->transform = transform * this->transform;
}

void GDT::SceneNode::resetTransform()
{
    transform = glm::mat4();
}

void GDT::SceneNode::update(float deltaTime)
{
    updateCurrent(deltaTime);
    updateChildren(deltaTime);

    attachChildren();
    detachChildren();
}

void GDT::SceneNode::forEach(std::function<void(SceneNode&)> function, bool includeThis, unsigned int maxDepth)
{
    forEach(function, includeThis, maxDepth, 0U);
}

void GDT::SceneNode::forEach(std::function<void(SceneNode&)> function, bool includeThis, unsigned int maxDepth, unsigned int currentDepth)
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

bool GDT::SceneNode::operator ==(const SceneNode& other) const
{
    return this == &other;
}

void GDT::SceneNode::detachSelf()
{
    if(parent != nullptr)
    {
        parent->detachChild(this);
    }
}

void GDT::SceneNode::draw() const
{
    drawCurrent(transform);
    drawChildren(transform);
}

void GDT::SceneNode::draw(glm::mat4 parentTransform) const
{
    glm::mat4 worldTransform = transform * parentTransform;
    drawCurrent(worldTransform);
    drawChildren(worldTransform);
}

void GDT::SceneNode::drawCurrent(glm::mat4 /*worldTransform*/) const
{}

void GDT::SceneNode::drawChildren(glm::mat4 worldTransform) const
{
    std::for_each(children.begin(), children.end(),
        [&worldTransform] (const GDT::SceneNode::Ptr& child) { child->draw(worldTransform); });
}

void GDT::SceneNode::updateCurrent(float /*deltaTime*/)
{}

void GDT::SceneNode::updateChildren(float deltaTime)
{
    std::for_each(children.begin(), children.end(),
        [&deltaTime] (GDT::SceneNode::Ptr& child) { child->update(deltaTime); });
}

void GDT::SceneNode::attachChildren()
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

void GDT::SceneNode::detachChildren()
{
    while(!detachRequests.empty())
    {
        SceneNode* ptr = detachRequests.back();
        auto found = std::find_if(children.begin(), children.end(), [&ptr] (const GDT::SceneNode::Ptr& pointer) {
            return pointer.get() == ptr;
        });

        assert(found != children.end());

        children.erase(found);
        detachRequests.pop_back();
    }
}

