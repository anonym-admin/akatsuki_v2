#pragma once

#include "common/CommonType.h"
#include "common/CommonVertex.h"
#include <Windows.h>

using std::map;
using std::string;
using std::vector;

struct ModelMeshData
{
    vector<Vertex_t> vertices;
    vector<SkinnedVertex_t> skinnedVertices;
    vector<AkU32> indices;

    string albedoTextureFilename = "";
    string emissiveTextureFilename = "";
    string heightTextureFilename = "";
    string normalTextureFilename = "";
    string metallicTextureFilename = "";
    string roughnessTextureFilename = "";
    string aoTextureFilename = "";
    string opacityTextureFilename = "";
};

struct AnimationClip 
{
    struct Key 
    {
        Vector3 pos = Vector3(0.0f);
        Vector3 scale = Vector3(1.0f);
        Quaternion rot = Quaternion();

        Matrix GetTransform() 
        {
            return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rot) * Matrix::CreateTranslation(pos);
        }
    };

    string name;              // Name of this animation clip
    vector<vector<Key>> keys; // m_key[boneIdx][frameIdx]
    int numChannels;          // Number of bones
    int numKeys;              // Number of frames of this animation clip
    double duration;          // Duration of animation in ticks
    double ticksPerSec;       // Frames per second
};

struct AnimationData {

    map<string, int32_t> boneNameToId; // 뼈 이름과 인덱스 정수
    vector<string> boneIdToName; // boneNameToId의 Id 순서대로 뼈 이름 저장
    vector<int32_t> boneParents; // 부모 뼈의 인덱스
    vector<Matrix> offsetMatrices;
    vector<Matrix> boneTransforms;
    vector<AnimationClip> clips;
    Matrix defaultTransform;
    Matrix rootTransform = Matrix();
    Matrix accumulatedRootTransform = Matrix();
    Vector3 prevPos = Vector3(0.0f);

    Matrix Get(int clipId, int boneId, int frame) 
    {
        return defaultTransform.Invert() * offsetMatrices[boneId] *
            boneTransforms[boneId] * defaultTransform;
    }

    void Update(int clipId, int frame) 
    {
        auto& clip = clips[clipId];

        for (int boneId = 0; boneId < boneTransforms.size(); boneId++) {
            auto& keys = clip.keys[boneId];

            // 주의: 모든 채널(뼈)이 frame 개수가 동일하진 않음

            const int parentIdx = boneParents[boneId];
            const Matrix parentMatrix = parentIdx >= 0
                ? boneTransforms[parentIdx]
                : accumulatedRootTransform;

            // keys.size()가 0일 경우에는 Identity 변환
            auto key = keys.size() > 0
                ? keys[frame % keys.size()]
                : AnimationClip::Key(); // key가 reference 아님

            // Root일 경우
            if (parentIdx < 0) {
                if (frame != 0) {
                    accumulatedRootTransform =
                        Matrix::CreateTranslation(key.pos - prevPos) *
                        accumulatedRootTransform;
                }
                else {
                    auto temp = accumulatedRootTransform.Translation();
                    temp.y = key.pos.y; // 높이 방향만 첫 프레임으로 보정
                    accumulatedRootTransform.Translation(temp);
                }

                prevPos = key.pos;
                key.pos = Vector3(0.0f); // 대신에 이동 취소
            }

            boneTransforms[boneId] = key.GetTransform() * parentMatrix;
        }
    }
};