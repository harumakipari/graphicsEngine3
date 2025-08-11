#include "EnemyMath.h"

#include <stdlib.h>
#include <random>

float Mathf::RandomRange(float min, float max)
{
	//0.0～1.0の間までのランダム値
	float value = static_cast<float>(rand()) / RAND_MAX;

	//min～maxまでのランダム値に変換
	return min + (max - min) * value;
}

//ランダムな整数を生成する関数
int Mathf::RandomInt(int min, int max)
{
	using namespace std;

	static random_device rd;
	static mt19937 gen(rd());
	uniform_int_distribution<> dis(min, max);
	return dis(gen);
}