#include "staticMesh.h"
#include "GeometricPrimitive.h"
#include "Graphics/Core/Shader.h"
#include "Engine/Utility/Win32Utils.h"

//wfopenを使うため
#include <fstream>
//c++17以上で使用可能
#include <filesystem>

//pdf14でテクスチャのロードするために
#include "Texture.h"

StaticMesh::StaticMesh(ID3D11Device* device, const wchar_t* objFilename)
{
    //頂点データとインデックスデータを格納するためのベクターを定義
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t currentIndex{ 0 };

    //位置と法線ベクトルを格納するためのベクターを定義
    std::vector<DirectX::XMFLOAT3> positions;
    std::vector<DirectX::XMFLOAT3> normals;
    std::vector<DirectX::XMFLOAT2> texCoords;
    std::vector<std::wstring> mtlFilenames;


    //OBJファイルを読み込むための入力ストリームを作成
    std::wifstream fin(objFilename);
    //ファイルが存在しない場合はアサートエラー
    _ASSERT_EXPR(fin, L"OBJ file not found.");
    wchar_t command[256];

    boundPos.minPosition = { FLT_MAX,FLT_MAX,FLT_MAX };
    boundPos.maxPosition = { -FLT_MAX,-FLT_MAX,-FLT_MAX };

    //ファイルから１行ずつコマンドを読み込み処理を行う
    while (fin)
    {
        fin >> command;

        //
        if (0 == wcscmp(command, L"v"))
        {
            float x, y, z;
            fin >> x >> y >> z;
            positions.push_back({ x,y,z });

            if (boundPos.maxPosition.x < x)
            {
                boundPos.maxPosition.x = x;
            }
            else if (boundPos.minPosition.x > x)
            {
                boundPos.minPosition.x = x;
            }
            if (boundPos.maxPosition.y < y)
            {
                boundPos.maxPosition.y = y;
            }
            else if (boundPos.minPosition.y > y)
            {
                boundPos.minPosition.y = y;
            }
            if (boundPos.maxPosition.z < z)
            {
                boundPos.maxPosition.z = z;
            }
            else if (boundPos.minPosition.z > z)
            {
                boundPos.minPosition.z = z;
            }

            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"vn"))
        {
            float i, j, k;
            fin >> i >> j >> k;
            normals.push_back({ i,j,k });
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"vt"))
        {
            float u, v;
            fin >> u >> v;
            //texCoords.push_back({ u,v });
            texCoords.push_back({ u,1.0f - v });
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"f"))
        {
            for (size_t i = 0; i < 3; i++)
            {
                Vertex vertex;
                size_t v, vt, vn;

                fin >> v;
                vertex.position = positions.at(v - 1);
                if (L'/' == fin.peek())
                {
                    fin.ignore(1);
                    if (L'/' != fin.peek())
                    {
                        fin >> vt;
                        vertex.texCoord = texCoords.at(vt - 1);
                    }
                    if (L'/' == fin.peek())
                    {
                        fin.ignore(1);
                        fin >> vn;
                        vertex.normal = normals.at(vn - 1);
                    }
                }
                vertices.push_back(vertex);
                indices.push_back(currentIndex++);
            }
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"mtllib"))
        {
            wchar_t mtllib[256];
            fin >> mtllib;
            mtlFilenames.push_back(mtllib);
        }
        else if (0 == wcscmp(command, L"usemtl"))
        {
            wchar_t usemtl[MAX_PATH]{ 0 };
            fin >> usemtl;
            subsets.push_back({ usemtl,static_cast<uint32_t>(indices.size()),0 });
        }
        else
        {
            fin.ignore(1024, L'\n');
        }
    }
    fin.close();


    //MTLファイルパーサー部の実装
    std::filesystem::path mtlFilename(objFilename);
    mtlFilename.replace_filename(std::filesystem::path(mtlFilenames[0]).filename());

    fin.open(mtlFilename);
    //_ASSERT_EXPR(fin, L"MTL file not found");

    //テクスチャのロード、シェーダーリソースビューオブジェクトの生成を行う
    D3D11_TEXTURE2D_DESC texture2dDesc{};
    //hr = LoadTextureFromFile(device, textureFileName.c_str(), shaderResourceView.GetAddressOf(), &texture2dDesc);
    if (!fin)//TEXTUREが無い時
    {
        for (Material& material : materials)
        {
            MakeDummyTexture(device, material.shaderResourceViews[0].GetAddressOf(), 0xFFFFFFFF, 16);
            MakeDummyTexture(device, material.shaderResourceViews[1].GetAddressOf(), 0xFFFFFFFF, 16);
        }
    }
    else
    {
        for (Material& material : materials)
        {
            LoadTextureFromFile(device, material.textureFileNames[0].c_str(), material.shaderResourceViews[0].GetAddressOf(), &texture2dDesc);
            LoadTextureFromFile(device, material.textureFileNames[1].c_str(), material.shaderResourceViews[1].GetAddressOf(), &texture2dDesc);
        }
    }
    //_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    while (fin)
    {
        fin >> command;
        if (0 == wcscmp(command, L"map_Kd"))
        {
            fin.ignore();
            wchar_t map_Kd[256];
            fin >> map_Kd;

            std::filesystem::path path(objFilename);
            path.replace_filename(std::filesystem::path(map_Kd).filename());
            //textureFileName = path;
            //materials.rbegin()->textureFileName = path;
            materials.rbegin()->textureFileNames[0] = path;
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"map_Ka"))
        {
            fin.ignore();
            wchar_t map_Ka[256];
            fin >> map_Ka;

            std::filesystem::path path(objFilename);
            path.replace_filename(std::filesystem::path(map_Ka).filename());
            //textureFileName = path;
            materials.rbegin()->textureFileNames[0] = path;
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"map_Ks"))
        {
            fin.ignore();
            wchar_t map_Ks[256];
            fin >> map_Ks;

            std::filesystem::path path(objFilename);
            path.replace_filename(std::filesystem::path(map_Ks).filename());
            //textureFileName = path;
            materials.rbegin()->textureFileNames[0] = path;
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"map_bump") || 0 == wcscmp(command, L"bump"))
        {
            fin.ignore();
            wchar_t mapBump[256];
            fin >> mapBump;
            std::filesystem::path path(objFilename);
            path.replace_filename(std::filesystem::path(mapBump).filename());
            materials.rbegin()->textureFileNames[1] = path;
            fin.ignore(1024, L'\n');
        }
        else if (0 == wcscmp(command, L"newmtl"))
        {
            fin.ignore();
            wchar_t newmtl[256];
            Material material;
            fin >> newmtl;
            material.name = newmtl;
            materials.push_back(material);
        }
        else if (0 == wcscmp(command, L"Kd"))
        {
            float r, g, b;
            fin >> r >> g >> b;
            materials.rbegin()->Kd = { r,g,b,1 };
            fin.ignore(1024, L'\n');
        }
        else
        {
            fin.ignore(1024, L'\n');
        }
    }
    fin.close();

    //ダミーマテリアルの追加
    if (materials.size() == 0)
    {
        for (const Subset& subset : subsets)
        {
            materials.push_back({ subset.usemtl });
        }
    }


    std::vector<Subset>::reverse_iterator iterator = subsets.rbegin();
    iterator->indexCount = static_cast<uint32_t>(indices.size() - iterator->indexStart);
    for (iterator = subsets.rbegin() + 1; iterator != subsets.rend(); ++iterator)
    {
        iterator->indexCount = (iterator - 1)->indexStart - iterator->indexStart;
    }

    CreateComBuffers(device, vertices.data(), vertices.size(), indices.data(), indices.size());

    //シェーダーオブジェクトを生成
    HRESULT hr{ S_OK };

    //入力レイアウトの生成
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[]
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
    };


    hr = CreateVsFromCSO(device, "./Shader/staticMeshVS.cso", vertexShader.GetAddressOf(),
        inputLayout.GetAddressOf(), inputElementDesc, ARRAYSIZE(inputElementDesc));
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    hr = CreatePsFromCSO(device, "./Shader/staticMeshPS.cso", pixelShader.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    //メッシュの最小値と最大値を取り出す
    for (int i = 0; i < positions.size(); i++)
    {


    }

    D3D11_BUFFER_DESC bufferDesc{};
    bufferDesc.ByteWidth = sizeof(Constants);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    hr = device->CreateBuffer(&bufferDesc, nullptr, constantBuffer.GetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void StaticMesh::CreateComBuffers(ID3D11Device* device, Vertex* vertices, size_t vertexCount,
    uint32_t* indices, size_t indexCount)
{
    HRESULT hr{ S_OK };

    D3D11_BUFFER_DESC bufferDesc{};
    D3D11_SUBRESOURCE_DATA subsorceData{};
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * vertexCount);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    subsorceData.pSysMem = vertices;
    subsorceData.SysMemPitch = 0;
    subsorceData.SysMemSlicePitch = 0;
    hr = device->CreateBuffer(&bufferDesc, &subsorceData, vertexBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indexCount);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    subsorceData.pSysMem = indices;
    hr = device->CreateBuffer(&bufferDesc, &subsorceData, indexBuffer.ReleaseAndGetAddressOf());
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

//箱メッシュ生成
void StaticMesh::CreateBoxMesh(ID3D11Device* device)
{
    Vertex vertices[24]{};
    //サイズが1.0の正立方体のデータを作成する（重心を原点にする）。
    // 正立方体のコントロールポイントは8個
    //前面
    vertices[0].position = { -0.5f,0.5f,-0.5f };    //右上
    vertices[0].normal = { 0,0,-1 };
    vertices[1].position = { 0.5f,0.5f,-0.5f };     //左上
    vertices[1].normal = { 0,0,-1 };
    vertices[2].position = { -0.5f,-0.5f,-0.5f };   //左下
    vertices[2].normal = { 0,0,-1 };
    vertices[3].position = { 0.5f,-0.5f,-0.5f };    //右下
    vertices[3].normal = { 0,0,-1 };
    //上面
    vertices[4].position = { -0.5f,0.5f,0.5f };     //左奥
    vertices[4].normal = { 0,1,0 };
    vertices[5].position = { 0.5f,0.5f,0.5f };      //右奥
    vertices[5].normal = { 0,1,0 };
    vertices[6].position = { -0.5f,0.5f,-0.5f };    //左手前
    vertices[6].normal = { 0,1,0 };
    vertices[7].position = { 0.5f,0.5f,-0.5f };     //右手前
    vertices[7].normal = { 0,1,0 };
    //右面
    vertices[8].position = { 0.5f,0.5f,0.5f };      //上奥
    vertices[8].normal = { 1,0,0 };
    vertices[9].position = { 0.5f,-0.5f,0.5f };     //下奥
    vertices[9].normal = { 1,0,0 };
    vertices[10].position = { 0.5f,0.5f,-0.5f };     //上手前
    vertices[10].normal = { 1,0,0 };
    vertices[11].position = { 0.5f,-0.5f,-0.5f };   //下手前
    vertices[11].normal = { 1,0,0 };
    //左面
    vertices[12].position = { -0.5f,0.5f,-0.5f };
    vertices[12].normal = { -1,0,0 };
    vertices[13].position = { -0.5f,0.5f,0.5f };
    vertices[13].normal = { -1,0,0 };
    vertices[14].position = { -0.5f,-0.5f,-0.5f };
    vertices[14].normal = { -1,0,0 };
    vertices[15].position = { -0.5f,-0.5f,0.5f };
    vertices[15].normal = { -1,0,0 };
    //底面
    vertices[16].position = { -0.5f,-0.5f,0.5f };
    vertices[16].normal = { 0,-1,0 };
    vertices[17].position = { 0.5f,-0.5f,0.5f };
    vertices[17].normal = { 0,-1,0 };
    vertices[18].position = { -0.5f,-0.5f,-0.5f };
    vertices[18].normal = { 0,-1,0 };
    vertices[19].position = { 0.5f,-0.5f,-0.5f };
    vertices[19].normal = { 0,-1,0 };
    //奥面
    vertices[20].position = { -0.5f,0.5f,0.5f };
    vertices[20].normal = { 0,0,1 };
    vertices[21].position = { 0.5f,0.5f,0.5f };
    vertices[21].normal = { 0,0,1 };
    vertices[22].position = { -0.5f,-0.5f,0.5f };
    vertices[22].normal = { 0,0,1 };
    vertices[23].position = { 0.5f,-0.5f,0.5f };
    vertices[23].normal = { 0,0,1 };


    // 1つのコントロールポイントの位置には法線の向きが違う頂点が３つあるため、頂点情報の
    // 総数は8x3の24個、頂点情報配列(vertices)に全ての頂点の位置、法線情報を格納する。

    uint32_t indices[36]{};

    //前面　左
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    //前面　右
    indices[3] = 1;
    indices[4] = 3;
    indices[5] = 2;
    //上面　左
    indices[6] = 4;
    indices[7] = 5;
    indices[8] = 6;
    //上面　右
    indices[9] = 5;
    indices[10] = 7;
    indices[11] = 6;
    //右面　手前
    indices[12] = 10;
    indices[13] = 8;
    indices[14] = 11;
    //右面　奥
    indices[15] = 8;
    indices[16] = 9;
    indices[17] = 11;
    //左面  手前
    indices[18] = 13;
    indices[19] = 12;
    indices[20] = 14;
    //左面  奥
    indices[21] = 13;
    indices[22] = 14;
    indices[23] = 15;
    //底面　左
    indices[24] = 16;
    indices[25] = 18;
    indices[26] = 17;
    //底面　右
    indices[27] = 17;
    indices[28] = 18;
    indices[29] = 19;
    //奥面　左
    indices[30] = 20;
    indices[31] = 22;
    indices[32] = 21;
    //奥面　右
    indices[33] = 21;
    indices[34] = 22;
    indices[35] = 23;

    // 正立方体は6面持ち、1つの面は2つの3角形ポリゴンで構成されるので3角形ポリゴンの総数は6x2=12個、 
    // 正立方体を描画するために12回の3角形ポリゴン描画が必要、よって参照される頂点情報は12x3=36回、 
    // 3角形ポリゴンが参照する頂点情報のインデックス（頂点番号）を描画順に配列（indices）に格納する。 
    // 時計回りが表面になるように格納すること。

    CreateComBuffers(device, vertices, 24, indices, 36);
}

void CreateBoundingBox(ID3D11Device* device)
{

}

void StaticMesh::Render(ID3D11DeviceContext* immediateContext,
    const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& materialColor)
{
    using namespace DirectX;
    uint32_t stride{ sizeof(Vertex) };
    uint32_t offset{ 0 };
    immediateContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    immediateContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    //プリミティブタイプおよびデータの順序に関する情報のバインド
    immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //入力レイアウトオブジェクトのバインド
    immediateContext->IASetInputLayout(inputLayout.Get());

    immediateContext->VSSetShader(vertexShader.Get(), nullptr, 0);
    immediateContext->PSSetShader(pixelShader.Get(), nullptr, 0);

    //シェーダーリソースのバインド
    //生成したシェーダーリソースビューをピクセルシェーダーにバインド
    for (const Material& material : materials)
    {
        immediateContext->PSSetShaderResources(0, 1, material.shaderResourceViews[0].GetAddressOf());
        immediateContext->PSSetShaderResources(1, 1, material.shaderResourceViews[1].GetAddressOf());

        Constants data{ world,materialColor };
        DirectX::XMStoreFloat4(&data.materialColor, DirectX::XMLoadFloat4(&materialColor) * DirectX::XMLoadFloat4(&material.Kd));
        immediateContext->UpdateSubresource(constantBuffer.Get(), 0, 0, &data, 0, 0);
        immediateContext->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

        //D3D11_BUFFER_DESC bufferDesc{};
        //indexBuffer->GetDesc(&bufferDesc);
        for (const Subset& subset : subsets)
        {
            if (material.name == subset.usemtl)
            {
                immediateContext->DrawIndexed(subset.indexCount, subset.indexStart, 0);
            }
        }
    }
}