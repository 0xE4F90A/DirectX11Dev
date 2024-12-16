#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wrl/client.h> // Microsoft::WRL::ComPtr用
#include <windows.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3DCompiler.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Direct3D 11 を使用したクラス
class MiniDirectX11
{
public:
	MiniDirectX11(HWND hwnd);
	~MiniDirectX11();

	void RenderFrame(); // 1フレームを描画するメソッド

private:
	void InitD3D(HWND hwnd); // Direct3D初期化
	void CleanD3D(); // Direct3Dクリーンアップ
	void CompileShaders(); // シェーダーコンパイル
	void CreateVertexBuffer(); // 頂点バッファ作成

	// Direct3D デバイス関連のメンバ変数
	ComPtr<ID3D11Device> device;					// Direct3Dデバイス
	ComPtr<ID3D11DeviceContext> context;			// Direct3Dデバイスコンテキスト
	ComPtr<IDXGISwapChain> swapChain;				// スワップチェーーーーーーーーーーーン
	ComPtr<ID3D11RenderTargetView> renderTargetView;// レンダーターゲットビュー

	// シェーダー関連
	ComPtr<ID3D11VertexShader> vertexShader;		// 頂点シェーダー
	ComPtr<ID3D11HullShader> hullShader;			// ハルシェーダー
	ComPtr<ID3D11DomainShader> domainShader;		// ドメインシェーダー
	ComPtr<ID3D11GeometryShader> geometryShader;	// ジオメトリシェーダー
	ComPtr<ID3D11PixelShader> pixelShader;			// ピクセルシェーダー
	ComPtr<ID3D11InputLayout> inputLayout;			// 入力レイアウト

	// 頂点バッファ
	ComPtr<ID3D11Buffer> vertexBuffer;				// 頂点バッファ
};

// Direct3D の初期化とシェーダー、頂点バッファの作成を行う
MiniDirectX11::MiniDirectX11(HWND hwnd)
{
	InitD3D(hwnd);			// Direct3D の初期化
	CompileShaders();		// シェーダーのコンパイル
	CreateVertexBuffer();	// 頂点バッファの作成
}

// リソースのクリーンアップを行う
MiniDirectX11::~MiniDirectX11()
{
	CleanD3D();				// Direct3D のクリーンアップ
}

// Direct3Dの初期化, 引数のHWNDは描画するウィンドウのハンドル(識別子)を指定するためのもの
void MiniDirectX11::InitD3D(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC scd{};						// スワップチェーンの設定
	/* scdという構造体を使って スワップチェーンの設定を行う
	 * DXGI_SWAP_CHAIN_DESCはスワップチェーンの設定を格納するための専用の構造体
	 * ={}で一様初期化している*/

	scd.BufferCount = 1;								// バッファの数
	/* スワップチェーン内のバッファの数を1に設定する
	 * バッファは描画データを保持するためのメモリのこと
	 * 1なら次の描画と入れ替えるだけになる
	 * 10なら10枚のスケッチブックに順番に描画していって順に入れ替えるイメージ
	 * 多くなればなるほど滑らかに繋がるが、10個も使うとメモリ使用量が増え、遅延が発生する可能性あり
	 * 通常は2とか3ぐらいに設定される(ダブルバッファ) */

	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // バッファのフォーマット(RGBA 8 ビット)
	/* バッファの色フォーマットをR8G8B8A8にしている
	 * R(赤)が8bit, G(緑)が8bit, B(青)が8bit, A(透明度)が8bit
	 * 各256段階で表現され、0～1の範囲で正規化される 
	 * 要は float4(0.5, 1.0, 0.0, 1.0) ってこと*/

	scd.BufferDesc.Width = 800;							// バッファの幅
	/* 描画バッファの幅を800pxにするだけ
	 * 作成するウィンドウサイズと同じにしないと、変に拡大縮小されたりしておかしくなる */

	scd.BufferDesc.Height = 600;						// バッファの高さ
	/* 描画バッファの高さを600pxにするだけ 
	 * 作成するウィンドウサイズと同じにしないと、変に拡大縮小されたりしておかしくなる */

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// バッファの使用方法(レンダリングターゲットとして使用)
	/* このバッファの使用目的を後ろで設定してる
	 * 今回の場合、レンダリングターゲット(描画結果を保持する目的)として使用する
	 * 他のプロパティはここに書いてある
	 * https://learn.microsoft.com/ja-jp/windows/win32/direct3ddxgi/dxgi-usage */

	scd.OutputWindow = hwnd;							// 出力先のウィンドウ
	/* どのウィンドウに描画を行うかをDirect3Dに伝えている */

	scd.SampleDesc.Count = 1;							// マルチサンプルの設定
	/* マルチサンプリングはアンチエイリアス処理の一種
	 * 1 の場合、アンチエイリアスを使わない設定になる */

	scd.Windowed = TRUE;								// ウィンドウモードで実行
	/* ウィンドウモード, TRUEにするとフルスクリーンではなくウィンドウ内での描画になる */


	// デバイスとスワップチェーーーーーーーーン作成
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		/* 使用するグラフィックスアダプタ(GPU)を指定する 
		 * nullptrを選択するとシステムにあるアダプタから最適なものを自動的に選択してくれる
		 * 複数のGPUがある場合に特定のGPUを選びたいときに使う */

		D3D_DRIVER_TYPE_HARDWARE,
		/* ドライバーの種類を指定する
		 * D3D_DRIVER_TYPE_HARDWARE としていすることで物理GPUを使用する。通常はこれを使う
		 * 他のはそんな使わないかな 正確だけど遅い描画処理を使うなら D3D_DRIVER_TYPE_REFERENCE (デバッグ用) */

		nullptr,
		/* ソフトウェアレンダリング用のモジュールハンドルを指定する
		 * 難しく見えるが、上のドライバ選択の時に D3D_DRIVER_TYPE_SOFTWARE を使う場合にのみ指定する
		 * 通常はnullptrで問題ない */

		0,
		/* デバイス作成におけるフラグを指定
		 * 複数のフラグを指定することもできるが、特別な要件がなければ0で問題なし */

		nullptr,
		/* デバイスがサポートするDirect3Dの機能レベルを指定する配列
		 * Direct3Dには複数の機能レベルがあって、古いGPUでも対応できるようにするための互換性設定がある
		 * 例えばD3D_FEATURE_LEVEL_11_0 とか 10_0 などあるが、nullptrにするとデフォルトの機能レベルで作成する */

		0,
		/* 上のパラメータで指定した機能レベルの配列の要素数を指定する
		 * 11_0と10_0の2つなら 2
		 * 今回は使っていないので 0 でOK */

		D3D11_SDK_VERSION,
		/* Direct3D SDKのバージョンを指定する 
		 * 常に D3D11_SDK_VERSION を指定する */

		&scd,
		/* スワップチェーンの設定を指定する構造体のポインタ 
		 * スワップチェーンは描画を管理するもので、上のDXGI_SWAP_CHAIN_DESC構造体でその詳細を設定したよね 
		 * ここにスワップチェーンの情報を指定することでDirect3Dデバイスとスワップチェーンが連携できる */

		&swapChain,
		/* 作成されたスワップチェーンのポインタを受け取るためのポインタ(ダブルポインタ)
		 * この引数でスワップチェーンオブジェクトが返され、これを使って描画結果を管理する
		 * 有効なポインタ変数を渡すこと。nullptrはダメ。絶対。*/

		&device,
		/* 作成されたDirect3Dデバイスのポインタを受け取るためのポインタ(ダブルポインタ)
		 * この引数でDirect3Dデバイスオブジェクトが返され、
		 * これを使って描画処理に必要なリソース（テクスチャ、バッファなど）を作成する */

		nullptr,
		/* 実際に作成されたデバイスがサポートする機能レベルを受け取るためのポインタ
		 * 指定した機能レベルの中で、どれがサポートされたのかを確認できるが、不要なので nullptr */

		&context);
		/* 作成されたデバイスコンテキストのポインタを受け取るためのポインタ(ダブルポインタ)
		 * ここで描画操作を行うためのデバイスコンテキストオブジェクトが返される 
		 * デバイスコンテキストとは...描画命令、例えば三角形を描画しろ！を送るためのもので、
		 * Direct3Dデバイスと連携して使用される */


	// バックバッファ取得
	ComPtr<ID3D11Texture2D> backBuffer;
	/* ComPtrはスマートポインタでDirect3Dオブジェを管理している
	 * ID3D11Texture2D は2Dテクスチャ(画像)を扱うためのインターフェース
	 * ここではバックバッファ(描画結果が一時的に保持されるバッファ)を格納するための変数を定義している */

	HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf());
	/* swapChain から描画するための「バックバッファ」を取得している 
	 * GetBuffer()で指定したバッファ(今回は0番目)を取得し、backBufferに格納する 
	 * 成功したかどうかは hr で確認できるので次の行でif文で見ている */

	if (SUCCEEDED(hr)) /* SUCCEEDED(hr)で成功すればtrueが返ってくる */
	{
		// バックバッファからレンダーターゲットビュー作成
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf());
		/* バックバッファから「レンダーターゲットビュー」を作成する
		 * レンダーターゲットビューは描画結果を表示するためのもので、
		 * renderTargetView に格納している */

		// レンダーターゲットを設定
		context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
		/* 出力マージャー(OM)ステージにレンダーターゲットビューを設定する
		 * この設定で描画結果がバックバッファに反映されるようになる */
	}

	// ビューポートの設定
	D3D11_VIEWPORT viewport{};
	/* ビューポート(描画領域)を設定するための構造体 */

	viewport.TopLeftX = 0;
	/* ビューポートの左上のX座標を0に設定する
	 * 描画が始まる位置を指定している */

	viewport.TopLeftY = 0;
	/* ビューポートの左上のY座標を0に設定する
	 * 描画が始まる位置を指定している */

	viewport.Width = 800;					// ビューポートの幅
	/* ビューポートの幅を800pxに設定する 描画する範囲 */

	viewport.Height = 600;					// ビューポートの高さ
	/* ビューポートの高さを600pxに設定する 描画する範囲 */

	viewport.MinDepth = 0.0f;				// 最小深度
	/* 最小深度を0.0fに設定する 深度は手前から奥行までの距離を表し、0が手前を意味する*/
	
	viewport.MaxDepth = 1.0f;				// 最大深度
	/* 最大深度を1.0fに設定する*/

	context->RSSetViewports(1, &viewport);	// ビューポートを設定
	/* ラスタライザーステージ(RS)にビューポートを設定する
	 * これでDirect3Dがどの範囲に描画を行うかが決まる */
}

// シェーダーのコンパイル作成
void MiniDirectX11::CompileShaders()
{
	/* ID3DBlob はシェーダーのコンパイル結果を格納するためのオブジェクト */
	ComPtr<ID3DBlob> shaderBlob;
	/* shaderBlob にはコンパイルされたシェーダーのデータが格納される */

	ComPtr<ID3DBlob> errorBlob;
	/* shaderBlob にはコンパイルが失敗した場合のエラーメッセージが格納される */

	// 頂点シェーダーのコンパイル
	HRESULT hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
	/* D3DCompileFromFile()は指定したHLSLファイルをコンパイルする
	 * 第1引数: シェーダーのパス
	 * 第2引数: マクロ定義のリファレンス 必要ないのでnullptr
	 * 第3引数: インクルードファイルのリファレンス 必要ないのでnullptr
	 * 第4引数: シェーダー内のエントリーポイント名 main なので main
	 * 第5引数: どのタイプのシェーダーかを指定 VertexShaderの5.0なのでこの書き方
	 * 第6引数: コンパイルのフラグ デフォルトでいいので 0
	 * 第7引数: コンパイルのフラグ デフォルトでいいので 0
	 * 第8引数: コンパイルされたシェーダーを受け取るポインタ
	 * 第9引数: コンパイルエラーを受け取るポインタ */

	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer()); // エラーメッセージ
		/* errorBlobのGetBufferPointer()でエラーメッセージの内容を取得し、キャストして表示する */
		return;
	}

	device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
	/* CreateVertexShader()はコンパイルされたシェーダーを使って頂点シェーダーを作成する 
	 * 第1引数: コンパイルされたシェーダーのデータを取得
	 * 第2引数: シェーダーデータのサイズを取得
	 * 第3引数: シェーダーのインターフェースに関連する設定を指定する 必要ないのでnullptr
	 * 第4引数: 作成した頂点シェーダーを格納するポインタ */

	// 入力レイアウト作成(頂点シェーダー入力形式定義)
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },};
	/* 頂点シェーダーに渡すデータの形式を定義 
	 * "POSITION": 頂点の位置情報を示す名前 シェーダー内で使用
	 * 0: このデータが何番目の要素なのかを示す
	 * DXGI_FORMAT_R32G32B32_FLOAT: 3つの32ビット浮動小数点で位置を表現するフォーマットx, y, zの3次元座標
	 * 0: 入力スロット
	 * 0: オフセット
	 * D3D11_INPUT_PER_VERTEX_DATA, 0: 頂点ごとにデータが渡される*/

	device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
	/* CreateInputLayout()で入力レイアウトを作成
	 * layoutDesc: 頂点データの形状の配列を渡す
	 * ARRAYSIZE(layoutDesc): 配列のサイズを指定
	 * shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(): さっきコンパイルされた頂点シェーダーのデータ
	 * inputLayout.GetAddressOf(): 作成した入力レイアウトを格納するポインタ */

	/*////あとは同じことの繰り返し////*/
	
	// ハルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"HullShader.hlsl", nullptr, nullptr, "main", "hs_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreateHullShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, hullShader.GetAddressOf());

	// ドメインシェーダーのコンパイル
	hr = D3DCompileFromFile(L"DomainShader.hlsl", nullptr, nullptr, "main", "ds_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreateDomainShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, domainShader.GetAddressOf());

	// ジオメトリシェーダーのコンパイル
	hr = D3DCompileFromFile(L"GeometryShader.hlsl", nullptr, nullptr, "main", "gs_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, geometryShader.GetAddressOf());

	// ピクセルシェーダーのコンパイル
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
}

// 頂点バッファ作成
void MiniDirectX11::CreateVertexBuffer()
{
	// 頂点データ定義
	struct Vertex {
		DirectX::XMFLOAT3 position;
		/* 3次元空間上の座標(x, y, z)を格納するための変数
		 * XMFLOAT3は3次元ベクトルを表す型 */
	};

	// 三角形の頂点データ(3頂点)
	Vertex vertices[] = {
		{ DirectX::XMFLOAT3(0.0f, 0.7f, 0.0f) },		// 上
		{ DirectX::XMFLOAT3(0.7f, -0.7f, 0.0f) },		// 右下
		{ DirectX::XMFLOAT3(-0.7f, -0.7f, 0.0f) }		// 左下
	};

	// 頂点バッファの作成
	D3D11_BUFFER_DESC bd{};
	/* 構造体、バッファの作成に関する詳細な設定を行う */

	bd.Usage = D3D11_USAGE_DEFAULT;				// デフォルト使用
	/* D3D11_USAGE_DEFAULTはGPUが読み書きするためのバッファで通常の使用ならこれ */

	bd.ByteWidth = sizeof(vertices);			// 頂点データサイズ
	/* バッファのサイズ 
	 * sizeof(vertices) はVertex構造体のサイズ * 3 になる */

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// 頂点バッファとして使用
	/* バッファの用途を指定する 
	 * 頂点バッファとして使用しよう！ */

	bd.CPUAccessFlags = 0;						// CPUはアクセスしない
	/* CPUからのアクセスについてのフラグを設定する 
	 * CPUからのアクセスを許可しない、つまりGPUのみがこのバッファを使用する */

	D3D11_SUBRESOURCE_DATA initData{};
	/* バッファを初期化するためのデータを設定する */

	initData.pSysMem = vertices;				// 頂点データ指定
	/* pSysMem はバッファを初期化するためのメモリのポインタでvertices(頂点データの配列)を渡す
	 * これで頂点バッファが作成されるときにverticesのデータで初期化される */

	// バッファ作成後 vertexBufferに格納
	device->CreateBuffer(&bd, &initData, vertexBuffer.GetAddressOf());
	/* 指定した設定に基づいて頂点バッファを作成
	 * &bd: バッファの設定情報が入ったD3D11_BUFFER_DESCのポインタ
	 * &initData: 初期化データが入ったD3D11_SUBRESOURCE_DATAのポインタ
	 * vertexBuffer.GetAddressOf(): 作成したバッファを格納するスマートポインタvertexBufferのアドレス */
}

// Direct3Dリソースをクリーンアップ
void MiniDirectX11::CleanD3D()
{
	// ComPtrは自動リソース解放されるので不要
}

// フレームレンダリング
void MiniDirectX11::RenderFrame()
{
	// 背景色(黒)設定
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	context->ClearRenderTargetView(renderTargetView.Get(), color); // バッファクリア
	/* ClearRenderTargetView()は指定した画面を指定した色で 塗りつぶす ような動作を行う
	 * これで前のフレームの描画が消え、次の描画を行うための準備が整う */

	// 入力アセンブラ(IA: Input Assembler)
	context->IASetInputLayout(inputLayout.Get());					// 入力レイアウト設定
	/* 入力アセンブラ（IA）に入力レイアウトを設定する
	 * 入力レイアウトは、頂点の位置情報やその他のデータのフォーマット(どのような形で頂点情報が格納されているか)を
	 * Direct3Dに教えるために使われる */

	// 頂点バッファバインド
	UINT stride = sizeof(XMFLOAT3);									// 各頂点のサイズ
	/* 各頂点のサイズを設定 
	 * strideは頂点データの「1つ分のサイズ」を表す
	 * XMFLOAT3は3つのfloat(x, y, z)で構成されているので、頂点1つ分のサイズはsizeof(XMFLOAT3)になる*/

	UINT offset = 0;												// バッファオフセット
	/*頂点バッファのオフセットを設定
	 * offsetは頂点バッファの先頭からの位置を指定
	 * 0を指定することで、バッファの先頭からデータを読み取るように指示する */

	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	/* 頂点バッファをバインド(GPUに関連付け)する
	 * 0：スロット番号、最初のスロットにバインドすることを示す
	 * 1：使用するバッファの数、ここでは1つの頂点バッファのみを使う
	 * vertexBuffer.GetAddressOf()：作成した頂点バッファのアドレスを指定
	 * &stride：各頂点のサイズを示すポインタ
	 * &offset：バッファの先頭から読み取る位置のオフセットを示すポインタ */

	// プリミティブトポロジ設定(ここでは三角形リスト)
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	/* プリミティブトポロジを設定
	 * D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST: 3つの頂点ごとに三角形を描画するという設定
	 * 頂点が順に3つずつ使われて三角形が描画される */

	// 頂点シェーダー設定
	context->VSSetShader(vertexShader.Get(), nullptr, 0);
	/* 頂点シェーダーを設定
	 * vertexShader.Get(): 頂点シェーダーを設定し、頂点データを処理する
	 * 頂点シェーダーは頂点の位置を計算し、座標変換(3D座標から2D画面座標への変換など)を行う役割がある */

	// 他シェーダー(ハル、ドメイン、ジオメトリ)は今回は使用しないため nullptr
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);

	// ピクセルシェーダー設定
	context->PSSetShader(pixelShader.Get(), nullptr, 0);

	// 描画コール(3頂点を使用して三角形描画)
	context->Draw(3, 0);
	/* 描画コールを行う
	 * 3は描画する頂点の数、ここでは三角形を描画するために3つの頂点を指定する
	 * 0は描画を開始する頂点のインデックス(最初の頂点)
	 * この命令でGPUが三角形の描画を行う */

	// スワップチェーンのバッファを交換して描画結果を画面に表示
	swapChain->Present(0, 0);
	/* バッファを交換して描画結果を画面に表示する
	 * Present()はバックバッファに描画した結果をフロントバッファと交換し、画面に表示する
	 * 0, 0は表示の方法を指定するオプションだが、ここではデフォルトの設定 */
}

// ウィンドウプロシージャのプロトタイプ宣言
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// WinMain: エントリーポイント
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// ウィンドウクラス設定
	WNDCLASSEX wc{};								// WNDCLASSEX構造体を初期化、全てのメンバ0
	/* ウィンドウクラスを定義するための構造体を初期化
	 * WNDCLASSEXはウィンドウのスタイルや、ウィンドウプロシージャ、アイコンなどを設定するための構造体*/

	wc.cbSize = sizeof(WNDCLASSEX);					// 構造体サイズ設定
	/* 構造体のサイズを設定
	 * cbSizeはこの構造体のサイズを設定するためのメンバで、必須の設定 */

	wc.style = CS_HREDRAW | CS_VREDRAW;				// ウィンドウが水平または垂直にサイズ変更された際に再描画するスタイル設定
	/* ウィンドウが再描画される条件を指定
	 * CS_HREDRAW | CS_VREDRAW は、ウィンドウの横幅または縦幅が変更されたときに再描画する設定 */

	wc.lpfnWndProc = WindowProc;					// ウィンドウプロシージャのポインタ設定、ウィンドウメッセージ処理関数
	/* ウィンドウメッセージを処理する関数のポインタを設定
	 * lpfnWndProc はウィンドウプロシージャのポインタで、
	 * ウィンドウがユーザーから受け取るメッセージ(クリックやキー入力など)を処理する関数を指定 */

	wc.hInstance = hInstance;						// 現在のアプリケーションインスタンス設定
	/* 現在のアプリケーションのインスタンスハンドルを設定 
	 * hInstanceは、このプログラムのインスタンス(実行中のプログラムを一意に識別するもの) */

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	// 標準の矢印カーソルをロードして設定
	/* カーソルを設定
	 * LoadCursorで標準の矢印カーソルをロードして設定する */

	wc.lpszClassName = L"WindowClass";				// ウィンドウクラス名設定
	/* ウィンドウクラスの名前を設定 
	 * ここで設定した名前を使って後でウィンドウを作成する
	 * L"WindowClass"は、このクラスを識別するための名前 */

	RegisterClassEx(&wc);							// 設定されたウィンドウクラス登録
	/* 設定したウィンドウクラスを登録 
	 * RegisterClassEx()はシステムにこのウィンドウクラスを登録し、後でウィンドウを作成するときに使用する */

	// ウィンドウサイズ設定
	RECT wr = { 0, 0, 800, 600 };
	/* ウィンドウのサイズを設定するための矩形(四角形)を定義
	 * 0, 0はウィンドウの左上の座標
	 * 800, 600はウィンドウの幅と高さ */

	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	/* ウィンドウのクライアント領域のサイズを適切に調整する 
	 * AdjustWindowRect()は指定されたスタイルに基づいて、実際のウィンドウサイズが指定したクライアント領域を持つように調整する
	 * WS_OVERLAPPEDWINDOWは、標準的なウィンドウスタイル */

	// ウィンドウ作成
	HWND hwnd = CreateWindowEx(
		0,
		/* 0は拡張スタイルで、今回は特に拡張は行わないので0 */

		L"WindowClass",
		/* さっき登録したウィンドウクラスの名前 */

		L"MiniDirectX11",
		/* ウィンドウのタイトル、画面上部に表示される名前 */

		WS_OVERLAPPEDWINDOW,
		/* WS_OVERLAPPEDWINDOWはウィンドウスタイルで、タイトルバーや最小化・最大化ボタンが含まれる */

		100,
		/* ウィンドウの表示位置、画面上のX座標 */

		100,
		/* ウィンドウの表示位置、画面上のY座標 */

		wr.right - wr.left,
		/* 調整された幅 */

		wr.bottom - wr.top,
		/* 調整された高さ */

		nullptr,
		/* 親ウィンドウ、無いのでnullptr */

		nullptr,
		/* メニュー、無いのでnullptr */

		hInstance,
		/* ウィンドウのインスタンスハンドル */

		nullptr);
		/* 作成時の追加情報、無いのでnullptr */


	ShowWindow(hwnd, nCmdShow);
	/* ウィンドウを表示 
	 * ShowWindow()は作成したウィンドウを表示するための関数で、
	 * nCmdShowはウィンドウの表示状態(通常の表示、最小化など)を指定する */

	// Direct3Dアプリのインスタンス作成
	MiniDirectX11 dxApp(hwnd);
	/* MiniDirectX11クラスのインスタンスを作成
	 * Direct3Dを使った描画のためのオブジェクトを作成し、ウィンドウハンドルhwndを渡して初期化を行う */

	MSG msg{};
	/* ウィンドウメッセージの構造体を定義し、初期化
	 * Windowsのイベントメッセージを格納するための構造体
	 * 例えば、ウィンドウの閉じるボタンが押された・キーが押されたなどを受け取るために使用する */

	while (msg.message != WM_QUIT) /* メッセージループを開始 */
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) /* メッセージがキューにあるかをチェックし、あれば取り出す */
		{
			TranslateMessage(&msg);
			/* 仮想キーコードを文字コードに変換
			 * メッセージがキー入力の場合、この関数で仮想キーコードを文字コードに変換して、
			 * 後でDispatchMessage()で処理できるようにする */

			DispatchMessage(&msg);
			/* メッセージをウィンドウプロシージャに送る 
			 * DispatchMessage()はmsgをWindowProc関数に送り、適切に処理する
			 * 例えばウィンドウの再描画や終了処理など */
		}
		else
		{
			// 毎フレーム描画
			dxApp.RenderFrame();
			/* 毎フレームの描画処理を呼び出す
			 * RenderFrame()はDirect3Dの描画を行う関数で、ここで画面の内容を更新する
			 * メッセージがない場合は、この描画処理を行うことで画面がスムーズに更新される */
		}
	}

	return static_cast<int>(msg.wParam);
	/* アプリケーションの終了コードを返す 
	 * msg.wParamにはアプリケーションの終了コードが格納されており、通常は0が返される */
}

// ウィンドウプロシージャ ウィンドウに対するメッセージを処理するためのもの
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		/* アプリケーションの終了を要求する
		 * PostQuitMessageはWM_QUITメッセージを送信し、メッセージループを終了するように指示する */
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
	/* 既定のウィンドウメッセージ処理を行う 
	 * 受け取ったメッセージが特に処理する必要のないものであれば、
	 * DefWindowProcが標準の動作、例えばウィンドウの移動やサイズ変更などを行う */
}
