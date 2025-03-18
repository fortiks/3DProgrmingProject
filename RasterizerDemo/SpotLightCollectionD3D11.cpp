#include "SpotLightCollectionD3D11.h"

void SpotLightCollectionD3D11::Initialize(ID3D11Device* device, const SpotLightData& lightInfo)
{
    bufferData.resize(lightInfo.perLightInfo.size());
    shadowCameras.resize(lightInfo.perLightInfo.size());

    for (size_t i = 0; i < lightInfo.perLightInfo.size(); ++i)
    {
        bufferData[i].colour = lightInfo.perLightInfo[i].colour;
        bufferData[i].angle = lightInfo.perLightInfo[i].angle;
        bufferData[i].position = lightInfo.perLightInfo[i].initialPosition;

        // direction
        DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(lightInfo.perLightInfo[i].rotationX);
        DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationY(lightInfo.perLightInfo[i].rotationY);
        DirectX::XMMATRIX rotation = rotationX * rotationY;
        DirectX::XMVECTOR direction = DirectX::XMVector3Transform(
            DirectX::XMVectorSet(0, -1, 1, 0), rotation);
        DirectX::XMStoreFloat3(&bufferData[i].direction, direction);

        ProjectionInfo projectionInfo;
        projectionInfo.fovAngleY = lightInfo.perLightInfo[i].angle * 2.0f;
        projectionInfo.aspectRatio = static_cast<float>(lightInfo.shadowMapInfo.textureDimension) / lightInfo.shadowMapInfo.textureDimension;
        projectionInfo.nearZ = lightInfo.perLightInfo[i].projectionNearZ;
        projectionInfo.farZ = lightInfo.perLightInfo[i].projectionFarZ;
       
        shadowCameras[i].Initialize(device, projectionInfo, lightInfo.perLightInfo[i].initialPosition);
        DirectX::XMFLOAT3 forward = { 0.0f, -1.0f, -1.0f };
        DirectX::XMStoreFloat3(&forward, direction);
        shadowCameras[i].SetForward(forward);
        bufferData[i].vpMatrix = shadowCameras[i].GetViewProjectionMatrix();

    }
    UINT textureDimension = lightInfo.shadowMapInfo.textureDimension;
    shadowMaps.Initialize(device, textureDimension, textureDimension, true, lightInfo.perLightInfo.size());
    lightBuffer.Initialize(device, sizeof(LightBuffer), bufferData.size(), bufferData.data(), true);

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(textureDimension);
    viewport.Height = static_cast<float>(textureDimension);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;

    std::optional<std::array<float, 4>> borderColor = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f};
    sampler.Initialize(device, D3D11_TEXTURE_ADDRESS_BORDER, borderColor);
}

void SpotLightCollectionD3D11::UpdateLightBuffers(ID3D11DeviceContext* context)
{
    lightBuffer.UpdateBuffer(context, bufferData.data());
    for (int index = 0; index < shadowCameras.size(); index++)
    {
        shadowCameras[index].UpdateInternalConstantBuffer(context);
    }

}

UINT SpotLightCollectionD3D11::GetNrOfLights() const
{
    return bufferData.size();
}

ID3D11DepthStencilView* SpotLightCollectionD3D11::GetShadowMapDSV(UINT lightIndex) const
{
    return shadowMaps.GetDSV(lightIndex);
}

ID3D11ShaderResourceView* SpotLightCollectionD3D11::GetShadowMapsSRV() const
{
    return shadowMaps.GetSRV();
}

ID3D11ShaderResourceView* SpotLightCollectionD3D11::GetLightBufferSRV() const
{
    return lightBuffer.GetSRV();
}

ID3D11Buffer* SpotLightCollectionD3D11::GetLightCameraConstantBuffer(UINT lightIndex) const
{
    return shadowCameras[lightIndex].GetConstantBuffer();
}

D3D11_VIEWPORT SpotLightCollectionD3D11::GetViewport() const
{
    return viewport;
}

void SpotLightCollectionD3D11::BindSpotlights(ID3D11DeviceContext* context)
{
    ID3D11ShaderResourceView* srv = lightBuffer.GetSRV();  // Get the SRV from your structured buffer
    if (srv)
    {
        context->CSSetShaderResources(3, 1, &srv); // Bind SRV to slot 3
    }
    ID3D11ShaderResourceView* srvShadowMap = shadowMaps.GetSRV();
    if (srvShadowMap)
    {
        context->CSSetShaderResources(4, 1, &srvShadowMap); // Bind SRV to slot 4
    }
    ID3D11SamplerState* shadowSampler = sampler.GetSamplerState();
    if (shadowSampler)
    {
        context->CSSetSamplers(0, 1, &shadowSampler);
    }
}

void SpotLightCollectionD3D11::UnBindSportLights(ID3D11DeviceContext* context)
{
    ID3D11ShaderResourceView* srv = nullptr;
    context->CSSetShaderResources(3, 1, &srv); // Bind SRV to slot 3
    ID3D11ShaderResourceView* srvShadowMap = nullptr;
    context->CSSetShaderResources(4, 1, &srvShadowMap); // Bind SRV to slot 4
}
