#pragma once
#include <d3d11.h>
#include <DirectxMath.h>

#include <vector>
#include <string>

class CollisionMesh
{
public:
	struct Mesh
	{
		std::string name;
		struct Subset
		{
			std::string materialName;
			std::vector<DirectX::XMFLOAT3> positions;
		};
		std::vector<Subset> subsets;
		DirectX::XMFLOAT3 boundingBox[2]
		{
			{ +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX, +D3D11_FLOAT32_MAX },
			{ -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX }
		};
	};
	std::vector<Mesh> meshes;
	// コンストラクタ: メッシュデータを読み込む
	CollisionMesh(ID3D11Device* device, const std::string& filename, bool triangulate = false);

	// レイキャスト: 与えられたレイとメッシュの交差を調べる// ワールド座標を渡す
	bool Raycast(_In_ DirectX::XMFLOAT3 ray_position, _In_ DirectX::XMFLOAT3 ray_direction, _In_ const DirectX::XMFLOAT4X4& transform, _Out_ DirectX::XMFLOAT3& intersection_position, _Out_ DirectX::XMFLOAT3& intersection_normal,
		_Out_ std::string& intersection_mesh, _Out_ std::string& intersection_material, _In_ float ray_length_limit = 1.0e+7f, _In_ bool skip_if = false/*Once the first intersection is found, the process is interrupted.*/) const;
};
