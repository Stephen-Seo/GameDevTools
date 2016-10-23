
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

    // disable copying
    SceneNode(const SceneNode& other) = delete;
    SceneNode& operator=(const SceneNode& other) = delete;

    /*!
        \brief Queues an append of a SceneNode::Ptr to the end of this node's
        children list.

        update() will actually perform the append.
    */
    void attachChild(Ptr child);
    /*!
        \brief Queues an insertion of a SceneNode::Ptr to the front of this
        node's children list.

        update() will actually perform the insertion.
    */
    void attachChildFront(Ptr child);
    /*!
        \brief Queues an append of a SceneNode::Ptr to the end of this tree's
        root's children list.

        update() will actually perform the append.
    */
    void attachToRoot(Ptr child);
    /*!
        \brief Queues an insertion of a SceneNode::Ptr to the front of this
        tree's root's children list.

        update() will actually perform the insertion.
    */
    void attachToRootFront(Ptr child);
    /*!
        \brief Queues a detach of a node from this node's children list.

        update() will actually perform the detach.
    */
    void detachChild(SceneNode* node);
    /*!
        \brief Immediately clears all child nodes from this node.
    */
    void clear();

    /*!
        \brief Returns the transform of this node.
    */
    const glm::mat4& getTransform() const;
    /*!
        \brief Returns the world transform of this node.

        If this node is the root node, then this function will return the same
        result as getTransform().
    */
    glm::mat4 getWorldTransform() const;
    /*!
        \brief Applies the given transform (matrix) to this node's transform
        (matrix).
    */
    void applyTransform(const glm::mat4& transform);
    /*!
        \brief Sets the current transform (matrix) to the identity matrix.
    */
    void resetTransform();

    /*!
        \brief Updates this node and all it's children with the given
        deltaTime.

        This base class does nothing on update (other than attach/detach
        nodes).
        Thus updateCurrent() must be overridden to perform an update using the
        given deltaTime.

        All queued attaches/detaches will occur after this update.
    */
    void update(float deltaTime);

    /*!
        \brief Calls a function with each/some SceneNodes in this tree.

        If parameter includeThis is false, then the given function will not
        be called with this node as a parameter.

        If parameter maxDepth is greater than 0, then the given function will
        only be called with the children at depth maxDepth or less.
        Note all children of the root is considered to be depth 1.
    */
    void forEach(std::function<void(SceneNode&)> function, bool includeThis = false, unsigned int maxDepth = 0U);

private:
    void forEach(std::function<void(SceneNode&)> function, bool includeThis, unsigned int maxDepth, unsigned int currentDepth);

public:
    /*!
        \brief Returns true if both SceneNodes occupy the same spot in memory.
    */
    bool operator ==(const SceneNode& other) const;

protected:
    /*!
        \brief Queues a detach of node from its parent, if this node has a
        parent.

        Note that the update function will perform all attaches/detaches
        queued.
    */
    void detachSelf();

public:
    /*!
        \brief Initiates a draw for the tree.

        The transform given to any node is the worldTransform of that node.

        This base class normally does nothing on draw.
        Thus drawCurrent() must be overridden to actually draw the current node
        using the given worldTransform.
    */
    void draw() const;

private:
    void draw(glm::mat4 parentTransform) const;
    virtual void drawCurrent(glm::mat4 worldTransform) const;
    void drawChildren(glm::mat4 worldTransform) const;

    virtual void updateCurrent(float deltaTime);
    void updateChildren(float deltaTime);

    void attachChildren();
    void detachChildren();

    std::list<Ptr>    children;
    std::vector<SceneNode::Ptr> attachRequests;
    std::vector<SceneNode::Ptr> attachRequestsFront;
    std::vector<SceneNode*> detachRequests;
    SceneNode*          parent;

    glm::mat4 transform;

};

#endif

