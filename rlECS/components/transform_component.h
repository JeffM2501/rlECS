/**********************************************************************************************
*
*   raylib_ECS_sample * a sample Entity Component System using raylib
*
*   LICENSE: ZLIB
*
*   Copyright (c) 2021 Jeffery Myers
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy
*   of this software and associated documentation files (the "Software"), to deal
*   in the Software without restriction, including without limitation the rights
*   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*   copies of the Software, and to permit persons to whom the Software is
*   furnished to do so, subject to the following conditions:
*
*   The above copyright notice and this permission notice shall be included in all
*   copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*   SOFTWARE.
*
**********************************************************************************************/

#pragma once

#include "entity_manager.h"

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include <vector>

class TransformComponent : public Component
{
private:
    Vector3 Position = { 0 };
    Quaternion Orientation = QuaternionIdentity();

    bool Dirty = true;

    Matrix WorldMatrix = { 0 };
    Matrix GlWorldMatrix = { 0 };

public:
    DEFINE_COMPONENT(TransformComponent);

    void Detach()
    {
        if (GetParent() == InvalidEntityId)
            return;

        Matrix worldTransform = GetWorldMatrix();
        Position = Vector3Transform(Vector3Zero(), WorldMatrix);

        Matrix translateMatrix = MatrixTranslate(Position.x, Position.y, Position.z);
        Matrix orientationMatrix = MatrixMultiply(worldTransform, translateMatrix);

        Orientation = QuaternionFromMatrix(WorldMatrix);

        Entities.ReparentEntity(EntityId, InvalidEntityId);
    }

    void SetDirty()
    {
         Dirty = true;
         for (EntityId_t childId : GetEntity().Children)
         {
             TransformComponent* childTransform = Entities.GetComponent<TransformComponent>(childId);
             if (childTransform != nullptr)
                 childTransform->SetDirty();
         }
    }

    const Vector3& GetPosition() const { return Position; }

    inline Quaternion GetOrientation()
    {
        return Orientation;
    }

    inline Vector3 GetEulerAngles()
    {
        return QuaternionToEuler(Orientation);
    }

    inline Vector3 GetForwardVector() const
    {
        return Vector3RotateByQuaternion(Vector3{ 0, 0, 1 }, Orientation);
    }

    inline Vector3 GetUpVector() const
    {
        return Vector3RotateByQuaternion(Vector3{ 0, 1, 0 }, Orientation);
    }

    inline Vector3 GetWorldPosition()
    {
        Matrix worldTransform = GetWorldMatrix();
        return Vector3Transform(Vector3Zero(), WorldMatrix);
    }

    inline Vector3 GetWorldTarget()
    {
        Matrix worldTransform = GetWorldMatrix();
        Vector3 pos = Vector3Transform(Vector3Zero(), WorldMatrix);

        Matrix translateMatrix = MatrixTranslate(Position.x, Position.y, Position.z);
        Matrix orientationMatrix = MatrixMultiply(worldTransform, translateMatrix);

        return Vector3Add(pos, Vector3Transform(Vector3{ 0 , 1 , 0 }, WorldMatrix));
    }

    void SetPosition(float x, float y, float z)
    {
        Position.x = x;
        Position.y = y;
        Position.z = z;
        SetDirty();
    }

    void SetPosition(const Vector3& pos)
    {
        Position = pos;
        SetDirty();
    }

    void SetOrientation(const Vector3& eulerAngles)
    {
        Vector3 angles = Vector3Scale(eulerAngles, DEG2RAD);
        Orientation = QuaternionFromEuler(angles.x, angles.y, angles.z);
        SetDirty();
    }

    bool IsDirty()
    {
        return Dirty;
    }

    void LookAt(const Vector3& target, const Vector3& up)
    {
        SetDirty();
        Matrix mat = MatrixLookAt(Position, target, up);
        Orientation = QuaternionFromMatrix(mat);
    }

    Matrix GetLocalMatrix()
    {
        Matrix orient = QuaternionToMatrix(Orientation);
        Matrix translation = MatrixTranslate(Position.x, Position.y, Position.z);

        return MatrixMultiply(MatrixInvert(orient), translation);
    }

    void UpdateWorldMatrix()
    {
        Matrix parentMatrix = MatrixIdentity();
        EntityId_t parentId = GetParent();

        if (parentId != InvalidEntityId && Entities.GetComponent<TransformComponent>(parentId) != nullptr)
            parentMatrix = Entities.GetComponent<TransformComponent>(parentId)->GetWorldMatrix();

        WorldMatrix = MatrixMultiply(GetLocalMatrix(), parentMatrix);
        GlWorldMatrix = MatrixTranspose(WorldMatrix);

        Dirty = false;
    }
    
    const Matrix& GetWorldMatrix()
    {
        if (!IsDirty())
            return WorldMatrix;

        UpdateWorldMatrix();
        return WorldMatrix;
    }

    const Matrix& GetGLWorldMatrix()
    {
        if (!IsDirty())
            return GlWorldMatrix;

        UpdateWorldMatrix();
        return GlWorldMatrix;
    }

    Vector3 ToLocalPos(const Vector3& inPos)
    {
        return Vector3Transform(inPos, MatrixInvert(GetWorldMatrix()));
    }

    Vector3 GetLeftVector()
    {
        return Vector3CrossProduct(GetUpVector(), GetForwardVector());
    }

    Vector3 GetRightVector()
    {
        return Vector3CrossProduct(GetForwardVector(), GetUpVector());
    }

    void MoveUp(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GetUpVector(), distance));
    }

    void MoveDown(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GetUpVector(), -distance));
    }

    void MoveForward(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GetForwardVector(), distance));
    }

    void MoveBackwards(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GetForwardVector(), -distance));
    }

    void MoveLeft(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(GetLeftVector(), distance));
    }

    void MoveRight(float distance)
    {
        MoveLeft(-distance);
    }

    void RotateYaw(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(0, -angle * DEG2RAD, 0);
        Orientation = QuaternionMultiply(Orientation, rot);
    }

    void RotatePitch(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(angle * DEG2RAD, 0, 0);
        Orientation = QuaternionMultiply(Orientation, rot);
    }

    void RotateRoll(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(0, 0, -angle * DEG2RAD);
        Orientation = QuaternionMultiply(Orientation, rot);
    }

    void RotateHeading(float angle)
    {
        SetDirty();
        auto rot = QuaternionFromEuler(0, -angle * DEG2RAD, 0);
        Orientation = QuaternionMultiply(rot, Orientation);
    }

    void PushMatrix()
    {
        const Matrix& glMatrix = GetGLWorldMatrix();
        rlPushMatrix();
        rlMultMatrixf((float*)(&glMatrix.m0));
    }

    void PopMatrix()
    {
        rlPopMatrix();
    }
};