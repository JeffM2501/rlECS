
#include "scene_editor.h"

using namespace std::placeholders;

SceneEditor::SceneEditor(SceneData& sceneData)
    :Scene(sceneData)
{
}

void SceneEditor::SetOutliner(std::shared_ptr<SceneOutliner> outliner)
{
    Outliner = outliner;

    outliner->CreateEntityCallback = std::bind(&SceneEditor::CreateEntity, this, _1);
}

void SceneEditor::CreateEntity(EntityId_t parentId)
{
    EntityId_t newEntity = Scene.Entities.CreateEntity();
    Entity* entity = Scene.Entities.GetEntity(newEntity);

    Scene.Entities.ReparentEntity(newEntity, parentId);

    entity->Name = GetUniqueEntityName("Entity", newEntity);
}

std::string SceneEditor::GetUniqueEntityName(const std::string& baseName, EntityId_t entityId)
{
    int count = 0;
    int max = 0;

    Scene.Entities.DoForEachSiblingEntity(entityId,
        [this, &max, &count, &baseName, &entityId](EntityId_t sibId)
        {
            const std::string& sibName = Scene.Entities.GetEntityName(sibId);

            if (sibName.rfind(baseName, 0) != 0)
                return;

            count++;
            if (sibName.size() == baseName.size())
                return;

            std::string sub = sibName.substr(baseName.size() + 1);
            if (sub[0] == '(' && sub[sub.size() - 1] == ')')
            {
                int val = atoi(sub.substr(1, sub.length() - 2).c_str());
                if (val > max)
                    max = val;
            }
        }
        );

    if (count == 0)
        return baseName;

    return baseName + " (" + std::to_string(max+1) + ")";
}
