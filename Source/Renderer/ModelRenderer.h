#pragma once

#include <vector>
#include "ModelComponen.h"
/*-------------------------------------
/*�����_���[�̏o�͏����̂܂Ƃ߃N���X
-------------------------------------*/

struct PipeLineStateObject
{

};


class Renderer
{
public:
	Renderer(class GameManager* gameManager);
	~Renderer();

	void Draw();

	void AddModel(class ModelComponen* spriteComponent);
	void RemoveModel(class ModelComponen* spriteComponent);

private:
    std::vector<ModelComponen*> models_;
};