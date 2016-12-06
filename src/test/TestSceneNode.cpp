
#include "gtest/gtest.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GDT/SceneNode.hpp>

void floatEqual(const float& given, const float& test)
{
    EXPECT_GT(given, test - 0.1f);
    EXPECT_LT(given, test + 0.1f);
}

class SNListener : public SceneNode
{
public:
    std::function<void(glm::mat4)> listener;

private:
    virtual void drawCurrent(glm::mat4 worldTransform) const override
    {
        listener(worldTransform);
    }
};

TEST(SceneNode, Transform)
{
    // setup
    // root
    SceneNode::Ptr rootPtr = SceneNode::Ptr(new SNListener());
    SNListener* root = static_cast<SNListener*>(rootPtr.get());
    root->applyTransform(glm::translate(glm::mat4(), glm::vec3(5.0f, 0.0f, 2.0f)));
    root->listener = [] (glm::mat4 worldTransform) {
        auto transformed = worldTransform * glm::vec4(0.0, 0.0, 0.0, 1.0f);
        floatEqual(transformed.x, 5.0f);
        floatEqual(transformed.y, 0.0f);
        floatEqual(transformed.z, 2.0f);
    };
    // child 1
    SceneNode::Ptr c1Ptr = SceneNode::Ptr(new SNListener());
    SNListener* c1 = static_cast<SNListener*>(c1Ptr.get());
    c1->applyTransform(glm::rotate(glm::mat4(), glm::acos(-1.0f) / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
    // root is now:
    // x = 0.0
    // y = 5.0
    // z = 2.0
    c1->applyTransform(glm::translate(glm::mat4(), glm::vec3(-10.0f, 5.0f, 0.0f)));
    c1->listener = [] (glm::mat4 worldTransform) {
        auto transformed = worldTransform * glm::vec4(0.0, 0.0, 0.0, 1.0f);
        floatEqual(transformed.x, -10.0f);
        floatEqual(transformed.y, 10.0f);
        floatEqual(transformed.z, 2.0f);
    };
    rootPtr->attachChild(std::move(c1Ptr));

    // child 2
    SceneNode::Ptr c2Ptr = SceneNode::Ptr(new SNListener());
    SNListener* c2 = static_cast<SNListener*>(c2Ptr.get());
    c2->applyTransform(glm::rotate(glm::mat4(), glm::acos(-1.0f) / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
    // c1 is now:
    // x = -10.0
    // y = -10.0
    // z =   2.0
    c2->applyTransform(glm::translate(glm::mat4(), glm::vec3(10.0f, 10.0f, -2.0f)));
    c2->listener = [] (glm::mat4 worldTransform) {
        auto transformed = worldTransform * glm::vec4(0.0, 0.0, 0.0, 1.0f);
        floatEqual(transformed.x, 0.0f);
        floatEqual(transformed.y, 0.0f);
        floatEqual(transformed.z, 0.0f);
    };
    c1->attachChild(std::move(c2Ptr));

    // actually perform attaches
    root->update(0.0f);

    // actually perform tests
    root->draw();
}

