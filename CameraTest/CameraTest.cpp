// CameraTest.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

// #2のコメントを追加
// なんと#4だった！

#include <stdio.h>

#include "snapscan_api.h"


// 設定値
static wchar_t path_snapscan_file[] = L"./resources/snapscan_dummy.xml";	// 初期設定ファイル
static int resolution[] = { 2048, 1088 };	// 解像度（変更後）
static double intrgration_time_ms = 1.6;	// インテグレーション時間（変更後）

// 共通変数
static HANDLE handle = 0x0;	// デバイスハンドル

static SystemProperties system_properties = { 0 };	// システムプロパティ
static ConfigurationParameters config = { 0 };		// 設定プロパティ
static RuntimeParameters runtime = { 0 };			// ランタイムプロパティ

static CubeDataFormat cube_format = { 0 };			//画像フォーマット
static CubeFloat reference_corrected = { 0 };		// ホワイトバランス
static CubeFloat cube_corrected = { 0 };			// 調整済み

static FrameFloat dark_reference = { 0 };			// ダークリファレンス
static CorrectionMatrix correction_matrix = { 0 };	// 補正マトリックス


// エラーメッセージ
static void PrintError(char const* i_caller_name, HSI_RETURN i_return_val)
{
	// simple pass-through
	if (i_return_val == HSI_OK)
	{
		return;
	}

	// translate the return code
	char error_msg[256];
	switch (i_return_val)
	{
	case HSI_HANDLE_INVALID:
	{
		sprintf_s(error_msg, 256, "Invalid device handle specified.");
		break;
	}
	case HSI_ARGUMENT_INVALID:
	{
		sprintf_s(error_msg, 256, "Invalid argument provided in function call.");
		break;
	}
	case HSI_CALL_ILLEGAL:
	{
		sprintf_s(error_msg, 256, "Function call illegal given the current snapscan state.");
		break;
	}
	case HSI_FILE_NOT_FOUND:
	{
		sprintf_s(error_msg, 256, "A file could not be found.");
		break;
	}
	case HSI_CALIBRATION_FILE_NOT_FOUND:
	{
		sprintf_s(error_msg, 256, "Sensor calibration file could not be found.");
		break;
	}
	case HSI_CONNECTION_FAILED:
	{
		sprintf_s(error_msg, 256, "Snapscan system could not be connected.");
		break;
	}
	case HSI_ALLOCATION_ERROR:
	{
		sprintf_s(error_msg, 256, "Allocation of resources failed.");
		break;
	}
	case HSI_ACQUISITION_FAILED:
	{
		sprintf_s(error_msg, 256, "Unable to fulfill acquisition.");
		break;
	}
	case HSI_DATA_NOT_ALLOCATED:
	{
		sprintf_s(error_msg, 256, "Provided data structure is not allocated.");
		break;
	}
	case HSI_DATA_NOT_VALID:
	{
		sprintf_s(error_msg, 256, "Data with valid flag false provided as input for operation.");
		break;
	}
	case HSI_DATA_NOT_COMPATIBLE:
	{
		sprintf_s(error_msg, 256, "Data provided is not compatible.");
		break;
	}
	case HSI_FILE_SYSTEM_ERROR:
	{
		sprintf_s(error_msg, 256, "Specified directory doesn't exist and could not be created.");
		break;
	}
	case HSI_FILE_IO_ERROR:
	{
		sprintf_s(error_msg, 256, "Could not read or write data from the filesystem.");
		break;
	}
	case HSI_INTERNAL_ERROR:
	{
		sprintf_s(error_msg, 256, "An undexpected internal error occurred.");
		break;
	}
	}

	// print error message
	printf("Error calling %s: %s\n", i_caller_name, error_msg);
}

// ログファイルの初期化
static int InitLog(void)
{
	HSI_RETURN return_val;

	return_val = commonInitializeLogger(L"./logs/", LV_WARNING);
	if (HSI_OK != return_val)
	{
		PrintError("InitializeLogger", return_val);
		return -1;
	}

	return 0;
}

// デバイスのオープン
static int OpenDevice(void)
{
	HSI_RETURN return_val;

	// デバイスのオープン
	return_val = OpenDevice(&handle, path_snapscan_file, true);
	if (HSI_OK != return_val)
	{
		PrintError("OpenDevice", return_val);
		return -1;
	}

	// システムプロパティの取得
	return_val = GetSystemProperties(handle, &system_properties);
	if (HSI_OK != return_val)
	{
		PrintError("GetSystemProperties", return_val);
		return -1;
	}

	// 設定プロパティの取得
	return_val = GetConfigurationParameters(handle, &config);
	if (HSI_OK != return_val)
	{
		PrintError("GetConfigurationParameters", return_val);
		return -1;
	}

	// ランタイムパラメータの取得
	return_val = GetRuntimeParameters(handle, &runtime);
	if (HSI_OK != return_val)
	{
		PrintError("GetRuntimeParameters", return_val);
		return -1;
	}

	return 0;
}

// 解像度の変更
static int ChangeResolution(int w, int h)
{
	HSI_RETURN return_val;

	// 解像度の設定
	config.cube_height = h;
	config.cube_width = w;

	// 設定パラメータの設定
	return_val = SetConfigurationParameters(handle, config);
	if (HSI_OK != return_val)
	{
		PrintError("SetConfigurationParameters", return_val);
		return -1;
	}

	return 0;
}

// 画像の初期化
static int InitCube(void)
{
	HSI_RETURN return_val;

	// カメラの初期化
	return_val = Initialize(handle);
	if (HSI_OK != return_val)
	{
		PrintError("Initialize", return_val);
		return -1;
	}

	// データフォーマットの取得
	return_val = GetOutputCubeDataFormat(handle, &cube_format);
	if (HSI_OK != return_val)
	{
		PrintError("GetOutputCubeDataFormat", return_val);
		return -1;
	}

	return 0;
}

// インテグレーション時間の変更
static int ChangeIntegrationTime(double t)
{
	HSI_RETURN return_val;

	// インテグレーション時間の設定
	runtime.integration_time_ms = t;

	// ランタイムパラメータの設定
	return_val = SetRuntimeParameters(handle, runtime);
	if (HSI_OK != return_val)
	{
		PrintError("SetRuntimeParameters", return_val);
		return -1;
	}

	return 0;
}

// 初期化
static int Init(void)
{
	// 準備
	InitLog();
	OpenDevice();

	// 解像度の変更
	ChangeResolution(resolution[0], resolution[1]);

	 // 画像の初期化
	InitCube();

	// インテグレーション時間の変更
	ChangeIntegrationTime(intrgration_time_ms);

	return 0;
}

// カメラの開始
static int CameraStart(void)
{
	HSI_RETURN return_val;

	// 開始
	return_val = Start(handle);
	if (HSI_OK != return_val)
	{
		PrintError("Start", return_val);
		return -1;
	}

	// ダークリファレンスの取得
	return_val = AcquireDarkReferenceFrame(handle, &dark_reference);
	if (HSI_OK != return_val)
	{
		PrintError("AcquireDarkReferenceFrame (dark reference)", return_val);
		return -1;
	}

	// 補正マトリックスの取得
	return_val = GetCorrectionMatrix(handle, &correction_matrix);
	if (HSI_OK != return_val)
	{
		PrintError("GetCorrectionMatrix", return_val);
		return -1;
	}

	return 0;
}

// ホワイトバランスの取得
static int GetWhiteBalance(void)
{
	HSI_RETURN return_val;

	// 画像領域の確保
	CubeFloat cube = { 0 };
	return_val = commonAllocateCube(&cube, cube_format);
	if (HSI_OK != return_val)
	{
		PrintError("AllocateCube (cube)", return_val);
		return -1;
	}

	// 画像の取得
	return_val = AcquireCube(handle, &dark_reference, &cube);
	if (HSI_OK != return_val)
	{
		PrintError("AcquireCube (reference)", return_val);
		return -1;
	}

	// 領域の確保
	return_val = AllocateCubeCorrected(&reference_corrected, correction_matrix, cube_format);
	if (HSI_OK != return_val)
	{
		PrintError("AllocateCubeCorrected", return_val);
		return -1;
	}

	// スペクトル補正を適用
	return_val = ApplySpectralCorrection(&reference_corrected, cube, correction_matrix);
	if (HSI_OK != return_val)
	{
		PrintError("ApplySpectralCorrection", return_val);
		return -1;
	}

	// ファイルに保存
	return_val = commonSaveCube(reference_corrected, L"./", L"reference_corrected", FF_ENVI);
	if (HSI_OK != return_val)
	{
		PrintError("SaveCube (cube)", return_val);
		return -1;
	}

	// 画像領域の解放
	return_val = commonDeallocateCube(&cube);
	if (HSI_OK != return_val)
	{
		PrintError("DeallocateCube (reference)", return_val);
		return -1;
	}

	return 0;
}

// 画像の取得
static int CameraScan(void)
{
	HSI_RETURN return_val;

	// 画像領域の確保
	CubeFloat cube = { 0 };
	return_val = commonAllocateCube(&cube, cube_format);
	if (HSI_OK != return_val)
	{
		PrintError("AllocateCube (cube)", return_val);
		return -1;
	}

	// 画像の取得
	return_val = AcquireCube(handle, &dark_reference, &cube);
	if (HSI_OK != return_val)
	{
		PrintError("AcquireCube (cube)", return_val);
		return -1;
	}

	// 領域の確保
	return_val = AllocateCubeCorrected(&cube_corrected, correction_matrix, cube_format);
	if (HSI_OK != return_val)
	{
		PrintError("AllocateCubeCorrected", return_val);
		return -1;
	}

	// スペクトル補正を適用
	return_val = ApplySpectralCorrection(&cube_corrected, cube, correction_matrix);
	if (HSI_OK != return_val)
	{
		PrintError("ApplySpectralCorrection", return_val);
		return -1;
	}

	// ホワイトバランス調整
	return_val = ApplyWhiteReference(&cube_corrected, cube_corrected, reference_corrected, 0.95);
	if (HSI_OK != return_val)
	{
		PrintError("Normalize", return_val);
		return -1;
	}

	// 画像領域の解放
	return_val = commonDeallocateCube(&cube);
	if (HSI_OK != return_val)
	{
		PrintError("DeallocateCube (cube)", return_val);
		return -1;
	}

	return 0;
}

// 画像の保存
static int SaveImage(void)
{
	HSI_RETURN return_val;

	// ファイルに保存
	return_val = commonSaveCube(cube_corrected, L"./", L"cube_corrected", FF_ENVI);
	if (HSI_OK != return_val)
	{
		PrintError("SaveCube (cube)", return_val);
		return -1;
	}

	return 0;
}

// カメラの終了
static int CameraEnd(void)
{
	HSI_RETURN return_val;

	// 画像領域の解放
	return_val = commonDeallocateCube(&cube_corrected);
	if (HSI_OK != return_val)
	{
		PrintError("DeallocateCube (cube)", return_val);
		return -1;
	}

	// ホワイトバランス領域の解放
	return_val = commonDeallocateCube(&reference_corrected);
	if (HSI_OK != return_val)
	{
		PrintError("DeallocateCube (reference)", return_val);
		return -1;
	}

	// ダークリファレンス領域の解放
	return_val = commonDeallocateFrame(&dark_reference);
	if (HSI_OK != return_val)
	{
		PrintError("DeallocateFrame (dark_reference)", return_val);
		return -1;
	}

	// 画像フォーマット領域の解放
	return_val = commonDeallocateCubeDataFormat(&cube_format);
	if (HSI_OK != return_val)
	{
		PrintError("DeallocateCubeDataFormat", return_val);
		return -1;
	}

	// 補正マトリックス領域の解放
	return_val = DeallocateCorrectionMatrix(&correction_matrix);
	if (HSI_OK != return_val)
	{
		PrintError("DeallocateCorrectionMatrix", return_val);
		return -1;
	}

	// デバイスのクローズ
	return_val = CloseDevice(&handle);
	if (HSI_OK != return_val)
	{
		PrintError("CloseDevice", return_val);
		return -1;
	}

	return 0;
}

int main()
{
	// 初期化
	if (Init() < 0)
	{
		return -1;
	}

	// カメラの準備
	if (CameraStart() < 0)
	{
		return -1;
	}

	// ホワイトバランスの取得
	printf("ホワイトリファレンスを取得します。\n");
	printf("準備が出来たら、何かキーを押して下さい。");
	{
		fflush(stdout);
		char buffer[2];
		fgets(buffer, 2, stdin);
	}
	printf("ホワイトリファレンスを取得しています...\n");
	if (GetWhiteBalance() < 0)
	{
		return -1;
	}

	// 画像の取得
	printf("画像を取得します。\n");
	printf("準備が出来たら、何かキーを押して下さい。");
	{
		fflush(stdout);
		char buffer[2];
		fgets(buffer, 2, stdin);
	}
	printf("画像を取得しています...\n");
	while (1)
	{
		// 画像の取得
		if (CameraScan() < 0)
		{
			return -1;
		}
		// 複数枚に対応できるようにする必要あり
		// データのサイズが大きい（1GB以上の予定）ので、一旦ファイルに保存したほうがいい
		// 毎回、保存するファイル名を変えるのがよいかと

		// カメラの移動
		// API-TOOL（WDM）を使って指示
		// scan->move->scan->move...の繰り返し
		// 最後までたどり着いたら、「もう動かせない」という返事があるはず
		// それを受けて、ループ終了

		break;	// 今は1回で終了
	}

	// 画像の統合
	// 複数枚の画像を1枚にする
	// 多少重なりがあるので、ピッタリくる位置を探す
	// （重なっている画素の差が一番小さいところ）

	// 画像の保存
	if (SaveImage() < 0)
	{
		return -1;
	}
	printf("画像を保存しました\n");

	// カメラの終了
	if (CameraEnd() < 0)
	{
		return -1;
	}

	return 0;
}
