#pragma once

//浮動小数算術
class Mathf
{
public:
	//指定範囲のランダム値を計算する
	static float RandomRange(float min, float max);

	//ランダムな整数を生成する関数
	static int RandomInt(int min, int max);
};
