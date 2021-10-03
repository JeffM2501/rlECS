#pragma once

#include "scene.h"
#include "outliner/scene_outliner.h"

#include <memory>

class SceneEditor
{
public:
    SceneEditor(SceneData& sceneData);
    void SetOutliner(std::shared_ptr<SceneOutliner> outliner);

    void CreateEntity(EntityId_t parentId);

protected:
    SceneData& Scene;
    std::shared_ptr<SceneOutliner> Outliner = nullptr;

protected:
    std::string GetUniqueEntityName(const std::string& baseName, EntityId_t entity);
};
