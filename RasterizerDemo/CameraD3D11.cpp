#include "CameraD3D11.h"

void CameraD3D11::MoveInDirection(float amount, const DirectX::XMFLOAT3& direction)
{
    DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&position);
    DirectX::XMVECTOR dirVec = DirectX::XMLoadFloat3(&direction);
    posVec = DirectX::XMVectorAdd(posVec, DirectX::XMVectorScale(dirVec, amount));
    DirectX::XMStoreFloat3(&position, posVec);
}

void CameraD3D11::RotateAroundAxis(float amount, const DirectX::XMFLOAT3& axis)
{
    DirectX::XMVECTOR axisVec = DirectX::XMLoadFloat3(&axis);
    DirectX::XMVECTOR forwardVec = DirectX::XMLoadFloat3(&forward);
    DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

    DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis(axisVec, amount);
    forwardVec = DirectX::XMVector3TransformNormal(forwardVec, rotationMatrix);
    upVec = DirectX::XMVector3TransformNormal(upVec, rotationMatrix);

    forwardVec = DirectX::XMVector3Normalize(forwardVec);
    upVec = DirectX::XMVector3Normalize(upVec);

    DirectX::XMStoreFloat3(&forward, forwardVec);
    DirectX::XMStoreFloat3(&up, upVec);

    DirectX::XMVECTOR rightVec = DirectX::XMVector3Cross(forwardVec, upVec);
    rightVec = DirectX::XMVector3Normalize(rightVec);
    DirectX::XMStoreFloat3(&right, rightVec);
}

CameraD3D11::CameraD3D11(ID3D11Device* device, const ProjectionInfo& projectionInfo, const DirectX::XMFLOAT3& initialPosition)
{
    Initialize(device, projectionInfo, initialPosition);
}

void CameraD3D11::Initialize(ID3D11Device* device, const ProjectionInfo& projectionInfo, const DirectX::XMFLOAT3& initialPosition)
{
    this->projInfo = projectionInfo;
    this->position = initialPosition;

    DirectX::XMFLOAT4X4 viewproj = GetViewProjectionMatrix();

    this->cameraBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4X4), &viewproj);
    

}

void CameraD3D11::MoveForward(float amount)
{
    MoveInDirection(amount, forward);
}

void CameraD3D11::MoveRight(float amount)
{
    MoveInDirection(amount, right);
}

void CameraD3D11::MoveUp(float amount)
{
    MoveInDirection(amount, up);
}

void CameraD3D11::RotateForward(float amount)
{
    RotateAroundAxis(amount, forward);
}

void CameraD3D11::RotateRight(float amount)
{
    RotateAroundAxis(amount, right);
}

void CameraD3D11::RotateUp(float amount)
{
    RotateAroundAxis(amount, up); 
}

void CameraD3D11::HandleMouseMovement(float deltaX, float deltaY, float sensitivity)
{
    // Sensitivity scaling
    deltaX *= sensitivity;
    deltaY *= sensitivity;

    // Yaw: Rotate around the up vector (horizontal movement)
    RotateAroundAxis(deltaX, up);

    // Pitch: Rotate around the right vector (vertical movement)
    RotateAroundAxis(-deltaY, right);

    // Re-normalize forward and up vectors to prevent floating-point inaccuracies
    DirectX::XMVECTOR forwardVec = DirectX::XMLoadFloat3(&forward);
    DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

    forwardVec = DirectX::XMVector3Normalize(forwardVec);
    upVec = DirectX::XMVector3Normalize(upVec);

    DirectX::XMStoreFloat3(&forward, forwardVec);
    DirectX::XMStoreFloat3(&up, upVec);

    // Update the right vector
    DirectX::XMVECTOR rightVec = DirectX::XMVector3Cross(forwardVec, upVec);
    DirectX::XMStoreFloat3(&right, rightVec);
}

const DirectX::XMFLOAT3& CameraD3D11::GetPosition() const
{
    return position;
}

const DirectX::XMFLOAT3& CameraD3D11::GetForward() const
{
    return forward;
}

const DirectX::XMFLOAT3& CameraD3D11::GetRight() const
{
    return right;
}

const DirectX::XMFLOAT3& CameraD3D11::GetUp() const
{
    return up;
}

const std::array<float, 3> CameraD3D11::GetFloatPosition()
{
    return { position.x, position.y, position.z };
}

void CameraD3D11::UpdateInternalConstantBuffer(ID3D11DeviceContext* context)
{
    DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(
        DirectX::XMLoadFloat3(&position),
        DirectX::XMLoadFloat3(&forward),
        DirectX::XMLoadFloat3(&up));

    DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
        projInfo.fovAngleY,
        projInfo.aspectRatio,
        projInfo.nearZ,
        projInfo.farZ);

    DirectX::XMFLOAT4X4 viewProjection;
    DirectX::XMStoreFloat4x4(&viewProjection, DirectX::XMMatrixTranspose(view * projection));
    //DirectX::XMStoreFloat4x4(&viewProjection, DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity()));
    this->cameraBuffer.UpdateBuffer(context, &viewProjection, sizeof(viewProjection));
}

void CameraD3D11::UpdateInternalOrthographicConstantBuffer(ID3D11DeviceContext* context, 
    DirectX::XMFLOAT3 sceneCenter, float shadowBoxSize,
    DirectX::XMFLOAT3 lightDir)
{
 
    position = sceneCenter;
    forward = lightDir;
    DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(
        DirectX::XMLoadFloat3(&position),
        DirectX::XMLoadFloat3(&forward),
        DirectX::XMLoadFloat3(&up));

    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(shadowBoxSize, shadowBoxSize,
            projInfo.nearZ, projInfo.farZ);

    DirectX::XMFLOAT4X4 viewProjection;
    DirectX::XMStoreFloat4x4(&viewProjection, DirectX::XMMatrixTranspose(view * projection));
    this->cameraBuffer.UpdateBuffer(context, &viewProjection, sizeof(viewProjection));
}


ID3D11Buffer* CameraD3D11::GetConstantBuffer() const
{
    return cameraBuffer.GetBuffer();
}

void CameraD3D11::SetForward(DirectX::XMFLOAT3 forward)
{
    this->forward = forward;
}

DirectX::XMFLOAT4X4 CameraD3D11::GetViewProjectionMatrix() const
{
    DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(
        DirectX::XMLoadFloat3(&position),
        DirectX::XMLoadFloat3(&forward),
        DirectX::XMLoadFloat3(&up));

    DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
        projInfo.fovAngleY,
        projInfo.aspectRatio,
        projInfo.nearZ,
        projInfo.farZ);

    DirectX::XMFLOAT4X4 viewProjection;
    DirectX::XMStoreFloat4x4(&viewProjection, DirectX::XMMatrixTranspose(view * projection));
    return viewProjection;
}

DirectX::XMFLOAT4X4 CameraD3D11::GetViewOrthographicProjectionMatrix(DirectX::XMFLOAT3 sceneCenter, float shadowBoxSize,
    DirectX::XMFLOAT3 lightDir) const
{
    DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(
        DirectX::XMLoadFloat3(&position),
        DirectX::XMLoadFloat3(&forward),
        DirectX::XMLoadFloat3(&up));

    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(shadowBoxSize, shadowBoxSize,
        projInfo.nearZ, projInfo.farZ);

    DirectX::XMFLOAT4X4 viewProjection;
    DirectX::XMStoreFloat4x4(&viewProjection, DirectX::XMMatrixTranspose(view * projection));
    return viewProjection;
}

DirectX::XMMATRIX CameraD3D11::GetViewMatrix() const
{
    DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(
        DirectX::XMLoadFloat3(&position),
        DirectX::XMLoadFloat3(&forward),
        DirectX::XMLoadFloat3(&up));

    return DirectX::XMMatrixTranspose(view);
}

DirectX::XMMATRIX CameraD3D11::GetProjectionMatrix() const
{
    DirectX::XMMATRIX projection = DirectX::XMMatrixPerspectiveFovLH(
        projInfo.fovAngleY,
        projInfo.aspectRatio,
        projInfo.nearZ,
        projInfo.farZ);

    

    return DirectX::XMMatrixTranspose(projection);
}

DirectX::XMFLOAT3 CameraD3D11::CalculateSceneCenter(float lightDistance) const
{
    DirectX::XMVECTOR posVec = XMLoadFloat3(&position);
    DirectX::XMVECTOR forwardVec = XMLoadFloat3(&forward);

    // Scale the forward vector.
    DirectX::XMVECTOR scaledForward = DirectX::XMVectorScale(forwardVec, lightDistance * 0.5f);

    // Add to get the new position.
    DirectX::XMVECTOR resultVec = DirectX::XMVectorAdd(posVec, scaledForward);

    // Store the result in an XMFLOAT3.
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, resultVec);

    return result;
}

float CameraD3D11::CalculateShadowBoxSize(float lightDistance) const
{
    float fov = projInfo.fovAngleY;  // Field of View in radians
    float aspectRatio = projInfo.aspectRatio;

    // Calculate frustum width & height at the shadow distance
    float halfHeight = tan(fov * 0.5f) * lightDistance;
    float halfWidth = halfHeight * aspectRatio;

    // The shadow box should at least cover this area
    return max(halfWidth, halfHeight) * 2.0f;
}
