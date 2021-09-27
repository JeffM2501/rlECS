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
    Vector3 Forward = { 0 , 0 , 1 };
    Vector3 Up = { 0, 1 , 0 };

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

        Forward = Vector3Transform(Vector3{ 0 , 1 , 0 }, WorldMatrix);
        Up = Vector3Transform(Vector3{ 0, 0 , 1 }, WorldMatrix);

        Entities.ReparentEntity(EntityId, InvalidEntityId);
    }

    const Vector3& GetPosition() const { return Position; }
    const Vector3& GetForwardVector() const { return Forward; }
    const Vector3& GetUpVector() const { return Up; }

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
        Dirty = true;
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

    bool IsDirty()
    {
        //EntityId_t parentId = GetParent();
       // if (parentId == InvalidEntityId || Entities.GetComponent<TransformComponent>(parentId) == nullptr)
            return Dirty;
        
       // TransformComponent* parentTransform = Entities.GetComponent<TransformComponent>(parentId);
       // return parentTransform->IsDirty() || Dirty;
    }

    void LookAt(const Vector3& target, const Vector3& up)
    {
        SetDirty();
        Forward = Vector3Normalize(Vector3Subtract(target, Position));
        Up = Vector3Normalize(up);
    }

    Matrix GetLocalMatrix()
    {
        Matrix orient = MatrixLookAt(Vector3Zero(), Forward, Up);
        // keep local Z up
        orient = MatrixMultiply(orient, MatrixRotateX(-90 * DEG2RAD));
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
        return Vector3CrossProduct(Up, Forward);
    }

    Vector3 GetRightVector()
    {
        return Vector3CrossProduct(Forward, Up);
    }

    void MoveUp(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(Up, distance));
    }

    void MoveDown(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(Up, -distance));
    }

    void MoveForward(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(Forward, distance));
    }

    void MoveBackwards(float distance)
    {
        SetDirty();
        Position = Vector3Add(Position, Vector3Scale(Forward, -distance));
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
        auto matrix = MatrixRotate(Up, DEG2RAD * angle);
        Forward = Vector3Normalize(Vector3Transform(Forward, matrix));
    }

    void RotatePitch(float angle)
    {
        SetDirty();

        angle = fmodf(angle, 360);
        if (angle < 0)
            angle += 360;
        else if (angle > 360)
            angle -= 360;

        auto matrix = MatrixRotate(GetLeftVector(), DEG2RAD * angle);

        Up = Vector3Normalize(Vector3Transform(Up, matrix));
        Forward = Vector3Normalize(Vector3Transform(Forward, matrix));
    }

    void RotateRoll(float angle)
    {
        SetDirty();
        auto matrix = MatrixRotate(Forward, DEG2RAD * angle);
        Up = Vector3Normalize(Vector3Transform(Up, matrix));
    }

    void RotateHeading(float angle)
    {
        SetDirty();
        Matrix matrix = MatrixRotateY(DEG2RAD * angle);
        Up = Vector3Normalize(Vector3Transform(Up, matrix));
        Forward = Vector3Normalize(Vector3Transform(Forward, matrix));
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