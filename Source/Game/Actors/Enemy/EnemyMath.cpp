#include "EnemyMath.h"

#include <stdlib.h>
#include <random>

float Mathf::RandomRange(float min, float max)
{
	//0.0�`1.0�̊Ԃ܂ł̃����_���l
	float value = static_cast<float>(rand()) / RAND_MAX;

	//min�`max�܂ł̃����_���l�ɕϊ�
	return min + (max - min) * value;
}

//�����_���Ȑ����𐶐�����֐�
int Mathf::RandomInt(int min, int max)
{
	using namespace std;

	static random_device rd;
	static mt19937 gen(rd());
	uniform_int_distribution<> dis(min, max);
	return dis(gen);
}