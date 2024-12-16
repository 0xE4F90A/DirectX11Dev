#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wrl/client.h> // Microsoft::WRL::ComPtr�p
#include <windows.h>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "D3DCompiler.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Direct3D 11 ���g�p�����N���X
class MiniDirectX11
{
public:
	MiniDirectX11(HWND hwnd);
	~MiniDirectX11();

	void RenderFrame(); // 1�t���[����`�悷�郁�\�b�h

private:
	void InitD3D(HWND hwnd); // Direct3D������
	void CleanD3D(); // Direct3D�N���[���A�b�v
	void CompileShaders(); // �V�F�[�_�[�R���p�C��
	void CreateVertexBuffer(); // ���_�o�b�t�@�쐬

	// Direct3D �f�o�C�X�֘A�̃����o�ϐ�
	ComPtr<ID3D11Device> device;					// Direct3D�f�o�C�X
	ComPtr<ID3D11DeviceContext> context;			// Direct3D�f�o�C�X�R���e�L�X�g
	ComPtr<IDXGISwapChain> swapChain;				// �X���b�v�`�F�[�[�[�[�[�[�[�[�[�[�[��
	ComPtr<ID3D11RenderTargetView> renderTargetView;// �����_�[�^�[�Q�b�g�r���[

	// �V�F�[�_�[�֘A
	ComPtr<ID3D11VertexShader> vertexShader;		// ���_�V�F�[�_�[
	ComPtr<ID3D11HullShader> hullShader;			// �n���V�F�[�_�[
	ComPtr<ID3D11DomainShader> domainShader;		// �h���C���V�F�[�_�[
	ComPtr<ID3D11GeometryShader> geometryShader;	// �W�I���g���V�F�[�_�[
	ComPtr<ID3D11PixelShader> pixelShader;			// �s�N�Z���V�F�[�_�[
	ComPtr<ID3D11InputLayout> inputLayout;			// ���̓��C�A�E�g

	// ���_�o�b�t�@
	ComPtr<ID3D11Buffer> vertexBuffer;				// ���_�o�b�t�@
};

// Direct3D �̏������ƃV�F�[�_�[�A���_�o�b�t�@�̍쐬���s��
MiniDirectX11::MiniDirectX11(HWND hwnd)
{
	InitD3D(hwnd);			// Direct3D �̏�����
	CompileShaders();		// �V�F�[�_�[�̃R���p�C��
	CreateVertexBuffer();	// ���_�o�b�t�@�̍쐬
}

// ���\�[�X�̃N���[���A�b�v���s��
MiniDirectX11::~MiniDirectX11()
{
	CleanD3D();				// Direct3D �̃N���[���A�b�v
}

// Direct3D�̏�����, ������HWND�͕`�悷��E�B���h�E�̃n���h��(���ʎq)���w�肷�邽�߂̂���
void MiniDirectX11::InitD3D(HWND hwnd)
{
	DXGI_SWAP_CHAIN_DESC scd{};						// �X���b�v�`�F�[���̐ݒ�
	/* scd�Ƃ����\���̂��g���� �X���b�v�`�F�[���̐ݒ���s��
	 * DXGI_SWAP_CHAIN_DESC�̓X���b�v�`�F�[���̐ݒ���i�[���邽�߂̐�p�̍\����
	 * ={}�ň�l���������Ă���*/

	scd.BufferCount = 1;								// �o�b�t�@�̐�
	/* �X���b�v�`�F�[�����̃o�b�t�@�̐���1�ɐݒ肷��
	 * �o�b�t�@�͕`��f�[�^��ێ����邽�߂̃������̂���
	 * 1�Ȃ玟�̕`��Ɠ���ւ��邾���ɂȂ�
	 * 10�Ȃ�10���̃X�P�b�`�u�b�N�ɏ��Ԃɕ`�悵�Ă����ď��ɓ���ւ���C���[�W
	 * �����Ȃ�΂Ȃ�قǊ��炩�Ɍq���邪�A10���g���ƃ������g�p�ʂ������A�x������������\������
	 * �ʏ��2�Ƃ�3���炢�ɐݒ肳���(�_�u���o�b�t�@) */

	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // �o�b�t�@�̃t�H�[�}�b�g(RGBA 8 �r�b�g)
	/* �o�b�t�@�̐F�t�H�[�}�b�g��R8G8B8A8�ɂ��Ă���
	 * R(��)��8bit, G(��)��8bit, B(��)��8bit, A(�����x)��8bit
	 * �e256�i�K�ŕ\������A0�`1�͈̔͂Ő��K������� 
	 * �v�� float4(0.5, 1.0, 0.0, 1.0) ���Ă���*/

	scd.BufferDesc.Width = 800;							// �o�b�t�@�̕�
	/* �`��o�b�t�@�̕���800px�ɂ��邾��
	 * �쐬����E�B���h�E�T�C�Y�Ɠ����ɂ��Ȃ��ƁA�ςɊg��k�����ꂽ�肵�Ă��������Ȃ� */

	scd.BufferDesc.Height = 600;						// �o�b�t�@�̍���
	/* �`��o�b�t�@�̍�����600px�ɂ��邾�� 
	 * �쐬����E�B���h�E�T�C�Y�Ɠ����ɂ��Ȃ��ƁA�ςɊg��k�����ꂽ�肵�Ă��������Ȃ� */

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// �o�b�t�@�̎g�p���@(�����_�����O�^�[�Q�b�g�Ƃ��Ďg�p)
	/* ���̃o�b�t�@�̎g�p�ړI�����Őݒ肵�Ă�
	 * ����̏ꍇ�A�����_�����O�^�[�Q�b�g(�`�挋�ʂ�ێ�����ړI)�Ƃ��Ďg�p����
	 * ���̃v���p�e�B�͂����ɏ����Ă���
	 * https://learn.microsoft.com/ja-jp/windows/win32/direct3ddxgi/dxgi-usage */

	scd.OutputWindow = hwnd;							// �o�͐�̃E�B���h�E
	/* �ǂ̃E�B���h�E�ɕ`����s������Direct3D�ɓ`���Ă��� */

	scd.SampleDesc.Count = 1;							// �}���`�T���v���̐ݒ�
	/* �}���`�T���v�����O�̓A���`�G�C���A�X�����̈��
	 * 1 �̏ꍇ�A�A���`�G�C���A�X���g��Ȃ��ݒ�ɂȂ� */

	scd.Windowed = TRUE;								// �E�B���h�E���[�h�Ŏ��s
	/* �E�B���h�E���[�h, TRUE�ɂ���ƃt���X�N���[���ł͂Ȃ��E�B���h�E���ł̕`��ɂȂ� */


	// �f�o�C�X�ƃX���b�v�`�F�[�[�[�[�[�[�[�[���쐬
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		/* �g�p����O���t�B�b�N�X�A�_�v�^(GPU)���w�肷�� 
		 * nullptr��I������ƃV�X�e���ɂ���A�_�v�^����œK�Ȃ��̂������I�ɑI�����Ă����
		 * ������GPU������ꍇ�ɓ����GPU��I�т����Ƃ��Ɏg�� */

		D3D_DRIVER_TYPE_HARDWARE,
		/* �h���C�o�[�̎�ނ��w�肷��
		 * D3D_DRIVER_TYPE_HARDWARE �Ƃ��Ă����邱�Ƃŕ���GPU���g�p����B�ʏ�͂�����g��
		 * ���̂͂���Ȏg��Ȃ����� ���m�����ǒx���`�揈�����g���Ȃ� D3D_DRIVER_TYPE_REFERENCE (�f�o�b�O�p) */

		nullptr,
		/* �\�t�g�E�F�A�����_�����O�p�̃��W���[���n���h�����w�肷��
		 * ��������邪�A��̃h���C�o�I���̎��� D3D_DRIVER_TYPE_SOFTWARE ���g���ꍇ�ɂ̂ݎw�肷��
		 * �ʏ��nullptr�Ŗ��Ȃ� */

		0,
		/* �f�o�C�X�쐬�ɂ�����t���O���w��
		 * �����̃t���O���w�肷�邱�Ƃ��ł��邪�A���ʂȗv�����Ȃ����0�Ŗ��Ȃ� */

		nullptr,
		/* �f�o�C�X���T�|�[�g����Direct3D�̋@�\���x�����w�肷��z��
		 * Direct3D�ɂ͕����̋@�\���x���������āA�Â�GPU�ł��Ή��ł���悤�ɂ��邽�߂̌݊����ݒ肪����
		 * �Ⴆ��D3D_FEATURE_LEVEL_11_0 �Ƃ� 10_0 �Ȃǂ��邪�Anullptr�ɂ���ƃf�t�H���g�̋@�\���x���ō쐬���� */

		0,
		/* ��̃p�����[�^�Ŏw�肵���@�\���x���̔z��̗v�f�����w�肷��
		 * 11_0��10_0��2�Ȃ� 2
		 * ����͎g���Ă��Ȃ��̂� 0 ��OK */

		D3D11_SDK_VERSION,
		/* Direct3D SDK�̃o�[�W�������w�肷�� 
		 * ��� D3D11_SDK_VERSION ���w�肷�� */

		&scd,
		/* �X���b�v�`�F�[���̐ݒ���w�肷��\���̂̃|�C���^ 
		 * �X���b�v�`�F�[���͕`����Ǘ�������̂ŁA���DXGI_SWAP_CHAIN_DESC�\���̂ł��̏ڍׂ�ݒ肵����� 
		 * �����ɃX���b�v�`�F�[���̏����w�肷�邱�Ƃ�Direct3D�f�o�C�X�ƃX���b�v�`�F�[�����A�g�ł��� */

		&swapChain,
		/* �쐬���ꂽ�X���b�v�`�F�[���̃|�C���^���󂯎�邽�߂̃|�C���^(�_�u���|�C���^)
		 * ���̈����ŃX���b�v�`�F�[���I�u�W�F�N�g���Ԃ���A������g���ĕ`�挋�ʂ��Ǘ�����
		 * �L���ȃ|�C���^�ϐ���n�����ƁBnullptr�̓_���B��΁B*/

		&device,
		/* �쐬���ꂽDirect3D�f�o�C�X�̃|�C���^���󂯎�邽�߂̃|�C���^(�_�u���|�C���^)
		 * ���̈�����Direct3D�f�o�C�X�I�u�W�F�N�g���Ԃ���A
		 * ������g���ĕ`�揈���ɕK�v�ȃ��\�[�X�i�e�N�X�`���A�o�b�t�@�Ȃǁj���쐬���� */

		nullptr,
		/* ���ۂɍ쐬���ꂽ�f�o�C�X���T�|�[�g����@�\���x�����󂯎�邽�߂̃|�C���^
		 * �w�肵���@�\���x���̒��ŁA�ǂꂪ�T�|�[�g���ꂽ�̂����m�F�ł��邪�A�s�v�Ȃ̂� nullptr */

		&context);
		/* �쐬���ꂽ�f�o�C�X�R���e�L�X�g�̃|�C���^���󂯎�邽�߂̃|�C���^(�_�u���|�C���^)
		 * �����ŕ`�摀����s�����߂̃f�o�C�X�R���e�L�X�g�I�u�W�F�N�g���Ԃ���� 
		 * �f�o�C�X�R���e�L�X�g�Ƃ�...�`�施�߁A�Ⴆ�ΎO�p�`��`�悵��I�𑗂邽�߂̂��̂ŁA
		 * Direct3D�f�o�C�X�ƘA�g���Ďg�p����� */


	// �o�b�N�o�b�t�@�擾
	ComPtr<ID3D11Texture2D> backBuffer;
	/* ComPtr�̓X�}�[�g�|�C���^��Direct3D�I�u�W�F���Ǘ����Ă���
	 * ID3D11Texture2D ��2D�e�N�X�`��(�摜)���������߂̃C���^�[�t�F�[�X
	 * �����ł̓o�b�N�o�b�t�@(�`�挋�ʂ��ꎞ�I�ɕێ������o�b�t�@)���i�[���邽�߂̕ϐ����`���Ă��� */

	HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)backBuffer.GetAddressOf());
	/* swapChain ����`�悷�邽�߂́u�o�b�N�o�b�t�@�v���擾���Ă��� 
	 * GetBuffer()�Ŏw�肵���o�b�t�@(�����0�Ԗ�)���擾���AbackBuffer�Ɋi�[���� 
	 * �����������ǂ����� hr �Ŋm�F�ł���̂Ŏ��̍s��if���Ō��Ă��� */

	if (SUCCEEDED(hr)) /* SUCCEEDED(hr)�Ő��������true���Ԃ��Ă��� */
	{
		// �o�b�N�o�b�t�@���烌���_�[�^�[�Q�b�g�r���[�쐬
		device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf());
		/* �o�b�N�o�b�t�@����u�����_�[�^�[�Q�b�g�r���[�v���쐬����
		 * �����_�[�^�[�Q�b�g�r���[�͕`�挋�ʂ�\�����邽�߂̂��̂ŁA
		 * renderTargetView �Ɋi�[���Ă��� */

		// �����_�[�^�[�Q�b�g��ݒ�
		context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);
		/* �o�̓}�[�W���[(OM)�X�e�[�W�Ƀ����_�[�^�[�Q�b�g�r���[��ݒ肷��
		 * ���̐ݒ�ŕ`�挋�ʂ��o�b�N�o�b�t�@�ɔ��f�����悤�ɂȂ� */
	}

	// �r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT viewport{};
	/* �r���[�|�[�g(�`��̈�)��ݒ肷�邽�߂̍\���� */

	viewport.TopLeftX = 0;
	/* �r���[�|�[�g�̍����X���W��0�ɐݒ肷��
	 * �`�悪�n�܂�ʒu���w�肵�Ă��� */

	viewport.TopLeftY = 0;
	/* �r���[�|�[�g�̍����Y���W��0�ɐݒ肷��
	 * �`�悪�n�܂�ʒu���w�肵�Ă��� */

	viewport.Width = 800;					// �r���[�|�[�g�̕�
	/* �r���[�|�[�g�̕���800px�ɐݒ肷�� �`�悷��͈� */

	viewport.Height = 600;					// �r���[�|�[�g�̍���
	/* �r���[�|�[�g�̍�����600px�ɐݒ肷�� �`�悷��͈� */

	viewport.MinDepth = 0.0f;				// �ŏ��[�x
	/* �ŏ��[�x��0.0f�ɐݒ肷�� �[�x�͎�O���牜�s�܂ł̋�����\���A0����O���Ӗ�����*/
	
	viewport.MaxDepth = 1.0f;				// �ő�[�x
	/* �ő�[�x��1.0f�ɐݒ肷��*/

	context->RSSetViewports(1, &viewport);	// �r���[�|�[�g��ݒ�
	/* ���X�^���C�U�[�X�e�[�W(RS)�Ƀr���[�|�[�g��ݒ肷��
	 * �����Direct3D���ǂ͈̔͂ɕ`����s���������܂� */
}

// �V�F�[�_�[�̃R���p�C���쐬
void MiniDirectX11::CompileShaders()
{
	/* ID3DBlob �̓V�F�[�_�[�̃R���p�C�����ʂ��i�[���邽�߂̃I�u�W�F�N�g */
	ComPtr<ID3DBlob> shaderBlob;
	/* shaderBlob �ɂ̓R���p�C�����ꂽ�V�F�[�_�[�̃f�[�^���i�[����� */

	ComPtr<ID3DBlob> errorBlob;
	/* shaderBlob �ɂ̓R���p�C�������s�����ꍇ�̃G���[���b�Z�[�W���i�[����� */

	// ���_�V�F�[�_�[�̃R���p�C��
	HRESULT hr = D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, shaderBlob.GetAddressOf(), errorBlob.GetAddressOf());
	/* D3DCompileFromFile()�͎w�肵��HLSL�t�@�C�����R���p�C������
	 * ��1����: �V�F�[�_�[�̃p�X
	 * ��2����: �}�N����`�̃��t�@�����X �K�v�Ȃ��̂�nullptr
	 * ��3����: �C���N���[�h�t�@�C���̃��t�@�����X �K�v�Ȃ��̂�nullptr
	 * ��4����: �V�F�[�_�[���̃G���g���[�|�C���g�� main �Ȃ̂� main
	 * ��5����: �ǂ̃^�C�v�̃V�F�[�_�[�����w�� VertexShader��5.0�Ȃ̂ł��̏�����
	 * ��6����: �R���p�C���̃t���O �f�t�H���g�ł����̂� 0
	 * ��7����: �R���p�C���̃t���O �f�t�H���g�ł����̂� 0
	 * ��8����: �R���p�C�����ꂽ�V�F�[�_�[���󂯎��|�C���^
	 * ��9����: �R���p�C���G���[���󂯎��|�C���^ */

	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer()); // �G���[���b�Z�[�W
		/* errorBlob��GetBufferPointer()�ŃG���[���b�Z�[�W�̓��e���擾���A�L���X�g���ĕ\������ */
		return;
	}

	device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
	/* CreateVertexShader()�̓R���p�C�����ꂽ�V�F�[�_�[���g���Ē��_�V�F�[�_�[���쐬���� 
	 * ��1����: �R���p�C�����ꂽ�V�F�[�_�[�̃f�[�^���擾
	 * ��2����: �V�F�[�_�[�f�[�^�̃T�C�Y���擾
	 * ��3����: �V�F�[�_�[�̃C���^�[�t�F�[�X�Ɋ֘A����ݒ���w�肷�� �K�v�Ȃ��̂�nullptr
	 * ��4����: �쐬�������_�V�F�[�_�[���i�[����|�C���^ */

	// ���̓��C�A�E�g�쐬(���_�V�F�[�_�[���͌`����`)
	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },};
	/* ���_�V�F�[�_�[�ɓn���f�[�^�̌`�����` 
	 * "POSITION": ���_�̈ʒu�����������O �V�F�[�_�[���Ŏg�p
	 * 0: ���̃f�[�^�����Ԗڂ̗v�f�Ȃ̂�������
	 * DXGI_FORMAT_R32G32B32_FLOAT: 3��32�r�b�g���������_�ňʒu��\������t�H�[�}�b�gx, y, z��3�������W
	 * 0: ���̓X���b�g
	 * 0: �I�t�Z�b�g
	 * D3D11_INPUT_PER_VERTEX_DATA, 0: ���_���ƂɃf�[�^���n�����*/

	device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), inputLayout.GetAddressOf());
	/* CreateInputLayout()�œ��̓��C�A�E�g���쐬
	 * layoutDesc: ���_�f�[�^�̌`��̔z���n��
	 * ARRAYSIZE(layoutDesc): �z��̃T�C�Y���w��
	 * shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(): �������R���p�C�����ꂽ���_�V�F�[�_�[�̃f�[�^
	 * inputLayout.GetAddressOf(): �쐬�������̓��C�A�E�g���i�[����|�C���^ */

	/*////���Ƃ͓������Ƃ̌J��Ԃ�////*/
	
	// �n���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"HullShader.hlsl", nullptr, nullptr, "main", "hs_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreateHullShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, hullShader.GetAddressOf());

	// �h���C���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"DomainShader.hlsl", nullptr, nullptr, "main", "ds_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreateDomainShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, domainShader.GetAddressOf());

	// �W�I���g���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"GeometryShader.hlsl", nullptr, nullptr, "main", "gs_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, geometryShader.GetAddressOf());

	// �s�N�Z���V�F�[�_�[�̃R���p�C��
	hr = D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, shaderBlob.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		return;
	}
	device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
}

// ���_�o�b�t�@�쐬
void MiniDirectX11::CreateVertexBuffer()
{
	// ���_�f�[�^��`
	struct Vertex {
		DirectX::XMFLOAT3 position;
		/* 3������ԏ�̍��W(x, y, z)���i�[���邽�߂̕ϐ�
		 * XMFLOAT3��3�����x�N�g����\���^ */
	};

	// �O�p�`�̒��_�f�[�^(3���_)
	Vertex vertices[] = {
		{ DirectX::XMFLOAT3(0.0f, 0.7f, 0.0f) },		// ��
		{ DirectX::XMFLOAT3(0.7f, -0.7f, 0.0f) },		// �E��
		{ DirectX::XMFLOAT3(-0.7f, -0.7f, 0.0f) }		// ����
	};

	// ���_�o�b�t�@�̍쐬
	D3D11_BUFFER_DESC bd{};
	/* �\���́A�o�b�t�@�̍쐬�Ɋւ���ڍׂȐݒ���s�� */

	bd.Usage = D3D11_USAGE_DEFAULT;				// �f�t�H���g�g�p
	/* D3D11_USAGE_DEFAULT��GPU���ǂݏ������邽�߂̃o�b�t�@�Œʏ�̎g�p�Ȃ炱�� */

	bd.ByteWidth = sizeof(vertices);			// ���_�f�[�^�T�C�Y
	/* �o�b�t�@�̃T�C�Y 
	 * sizeof(vertices) ��Vertex�\���̂̃T�C�Y * 3 �ɂȂ� */

	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// ���_�o�b�t�@�Ƃ��Ďg�p
	/* �o�b�t�@�̗p�r���w�肷�� 
	 * ���_�o�b�t�@�Ƃ��Ďg�p���悤�I */

	bd.CPUAccessFlags = 0;						// CPU�̓A�N�Z�X���Ȃ�
	/* CPU����̃A�N�Z�X�ɂ��Ẵt���O��ݒ肷�� 
	 * CPU����̃A�N�Z�X�������Ȃ��A�܂�GPU�݂̂����̃o�b�t�@���g�p���� */

	D3D11_SUBRESOURCE_DATA initData{};
	/* �o�b�t�@�����������邽�߂̃f�[�^��ݒ肷�� */

	initData.pSysMem = vertices;				// ���_�f�[�^�w��
	/* pSysMem �̓o�b�t�@�����������邽�߂̃������̃|�C���^��vertices(���_�f�[�^�̔z��)��n��
	 * ����Œ��_�o�b�t�@���쐬�����Ƃ���vertices�̃f�[�^�ŏ���������� */

	// �o�b�t�@�쐬�� vertexBuffer�Ɋi�[
	device->CreateBuffer(&bd, &initData, vertexBuffer.GetAddressOf());
	/* �w�肵���ݒ�Ɋ�Â��Ē��_�o�b�t�@���쐬
	 * &bd: �o�b�t�@�̐ݒ��񂪓�����D3D11_BUFFER_DESC�̃|�C���^
	 * &initData: �������f�[�^��������D3D11_SUBRESOURCE_DATA�̃|�C���^
	 * vertexBuffer.GetAddressOf(): �쐬�����o�b�t�@���i�[����X�}�[�g�|�C���^vertexBuffer�̃A�h���X */
}

// Direct3D���\�[�X���N���[���A�b�v
void MiniDirectX11::CleanD3D()
{
	// ComPtr�͎������\�[�X��������̂ŕs�v
}

// �t���[�������_�����O
void MiniDirectX11::RenderFrame()
{
	// �w�i�F(��)�ݒ�
	float color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	context->ClearRenderTargetView(renderTargetView.Get(), color); // �o�b�t�@�N���A
	/* ClearRenderTargetView()�͎w�肵����ʂ��w�肵���F�� �h��Ԃ� �悤�ȓ�����s��
	 * ����őO�̃t���[���̕`�悪�����A���̕`����s�����߂̏��������� */

	// ���̓A�Z���u��(IA: Input Assembler)
	context->IASetInputLayout(inputLayout.Get());					// ���̓��C�A�E�g�ݒ�
	/* ���̓A�Z���u���iIA�j�ɓ��̓��C�A�E�g��ݒ肷��
	 * ���̓��C�A�E�g�́A���_�̈ʒu���₻�̑��̃f�[�^�̃t�H�[�}�b�g(�ǂ̂悤�Ȍ`�Œ��_��񂪊i�[����Ă��邩)��
	 * Direct3D�ɋ����邽�߂Ɏg���� */

	// ���_�o�b�t�@�o�C���h
	UINT stride = sizeof(XMFLOAT3);									// �e���_�̃T�C�Y
	/* �e���_�̃T�C�Y��ݒ� 
	 * stride�͒��_�f�[�^�́u1���̃T�C�Y�v��\��
	 * XMFLOAT3��3��float(x, y, z)�ō\������Ă���̂ŁA���_1���̃T�C�Y��sizeof(XMFLOAT3)�ɂȂ�*/

	UINT offset = 0;												// �o�b�t�@�I�t�Z�b�g
	/*���_�o�b�t�@�̃I�t�Z�b�g��ݒ�
	 * offset�͒��_�o�b�t�@�̐擪����̈ʒu���w��
	 * 0���w�肷�邱�ƂŁA�o�b�t�@�̐擪����f�[�^��ǂݎ��悤�Ɏw������ */

	context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	/* ���_�o�b�t�@���o�C���h(GPU�Ɋ֘A�t��)����
	 * 0�F�X���b�g�ԍ��A�ŏ��̃X���b�g�Ƀo�C���h���邱�Ƃ�����
	 * 1�F�g�p����o�b�t�@�̐��A�����ł�1�̒��_�o�b�t�@�݂̂��g��
	 * vertexBuffer.GetAddressOf()�F�쐬�������_�o�b�t�@�̃A�h���X���w��
	 * &stride�F�e���_�̃T�C�Y�������|�C���^
	 * &offset�F�o�b�t�@�̐擪����ǂݎ��ʒu�̃I�t�Z�b�g�������|�C���^ */

	// �v���~�e�B�u�g�|���W�ݒ�(�����ł͎O�p�`���X�g)
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	/* �v���~�e�B�u�g�|���W��ݒ�
	 * D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST: 3�̒��_���ƂɎO�p�`��`�悷��Ƃ����ݒ�
	 * ���_������3���g���ĎO�p�`���`�悳��� */

	// ���_�V�F�[�_�[�ݒ�
	context->VSSetShader(vertexShader.Get(), nullptr, 0);
	/* ���_�V�F�[�_�[��ݒ�
	 * vertexShader.Get(): ���_�V�F�[�_�[��ݒ肵�A���_�f�[�^����������
	 * ���_�V�F�[�_�[�͒��_�̈ʒu���v�Z���A���W�ϊ�(3D���W����2D��ʍ��W�ւ̕ϊ��Ȃ�)���s������������ */

	// ���V�F�[�_�[(�n���A�h���C���A�W�I���g��)�͍���͎g�p���Ȃ����� nullptr
	context->HSSetShader(nullptr, nullptr, 0);
	context->DSSetShader(nullptr, nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);

	// �s�N�Z���V�F�[�_�[�ݒ�
	context->PSSetShader(pixelShader.Get(), nullptr, 0);

	// �`��R�[��(3���_���g�p���ĎO�p�`�`��)
	context->Draw(3, 0);
	/* �`��R�[�����s��
	 * 3�͕`�悷�钸�_�̐��A�����ł͎O�p�`��`�悷�邽�߂�3�̒��_���w�肷��
	 * 0�͕`����J�n���钸�_�̃C���f�b�N�X(�ŏ��̒��_)
	 * ���̖��߂�GPU���O�p�`�̕`����s�� */

	// �X���b�v�`�F�[���̃o�b�t�@���������ĕ`�挋�ʂ���ʂɕ\��
	swapChain->Present(0, 0);
	/* �o�b�t�@���������ĕ`�挋�ʂ���ʂɕ\������
	 * Present()�̓o�b�N�o�b�t�@�ɕ`�悵�����ʂ��t�����g�o�b�t�@�ƌ������A��ʂɕ\������
	 * 0, 0�͕\���̕��@���w�肷��I�v�V���������A�����ł̓f�t�H���g�̐ݒ� */
}

// �E�B���h�E�v���V�[�W���̃v���g�^�C�v�錾
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// WinMain: �G���g���[�|�C���g
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	// �E�B���h�E�N���X�ݒ�
	WNDCLASSEX wc{};								// WNDCLASSEX�\���̂��������A�S�Ẵ����o0
	/* �E�B���h�E�N���X���`���邽�߂̍\���̂�������
	 * WNDCLASSEX�̓E�B���h�E�̃X�^�C����A�E�B���h�E�v���V�[�W���A�A�C�R���Ȃǂ�ݒ肷�邽�߂̍\����*/

	wc.cbSize = sizeof(WNDCLASSEX);					// �\���̃T�C�Y�ݒ�
	/* �\���̂̃T�C�Y��ݒ�
	 * cbSize�͂��̍\���̂̃T�C�Y��ݒ肷�邽�߂̃����o�ŁA�K�{�̐ݒ� */

	wc.style = CS_HREDRAW | CS_VREDRAW;				// �E�B���h�E�������܂��͐����ɃT�C�Y�ύX���ꂽ�ۂɍĕ`�悷��X�^�C���ݒ�
	/* �E�B���h�E���ĕ`�悳���������w��
	 * CS_HREDRAW | CS_VREDRAW �́A�E�B���h�E�̉����܂��͏c�����ύX���ꂽ�Ƃ��ɍĕ`�悷��ݒ� */

	wc.lpfnWndProc = WindowProc;					// �E�B���h�E�v���V�[�W���̃|�C���^�ݒ�A�E�B���h�E���b�Z�[�W�����֐�
	/* �E�B���h�E���b�Z�[�W����������֐��̃|�C���^��ݒ�
	 * lpfnWndProc �̓E�B���h�E�v���V�[�W���̃|�C���^�ŁA
	 * �E�B���h�E�����[�U�[����󂯎�郁�b�Z�[�W(�N���b�N��L�[���͂Ȃ�)����������֐����w�� */

	wc.hInstance = hInstance;						// ���݂̃A�v���P�[�V�����C���X�^���X�ݒ�
	/* ���݂̃A�v���P�[�V�����̃C���X�^���X�n���h����ݒ� 
	 * hInstance�́A���̃v���O�����̃C���X�^���X(���s���̃v���O��������ӂɎ��ʂ������) */

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);	// �W���̖��J�[�\�������[�h���Đݒ�
	/* �J�[�\����ݒ�
	 * LoadCursor�ŕW���̖��J�[�\�������[�h���Đݒ肷�� */

	wc.lpszClassName = L"WindowClass";				// �E�B���h�E�N���X���ݒ�
	/* �E�B���h�E�N���X�̖��O��ݒ� 
	 * �����Őݒ肵�����O���g���Č�ŃE�B���h�E���쐬����
	 * L"WindowClass"�́A���̃N���X�����ʂ��邽�߂̖��O */

	RegisterClassEx(&wc);							// �ݒ肳�ꂽ�E�B���h�E�N���X�o�^
	/* �ݒ肵���E�B���h�E�N���X��o�^ 
	 * RegisterClassEx()�̓V�X�e���ɂ��̃E�B���h�E�N���X��o�^���A��ŃE�B���h�E���쐬����Ƃ��Ɏg�p���� */

	// �E�B���h�E�T�C�Y�ݒ�
	RECT wr = { 0, 0, 800, 600 };
	/* �E�B���h�E�̃T�C�Y��ݒ肷�邽�߂̋�`(�l�p�`)���`
	 * 0, 0�̓E�B���h�E�̍���̍��W
	 * 800, 600�̓E�B���h�E�̕��ƍ��� */

	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	/* �E�B���h�E�̃N���C�A���g�̈�̃T�C�Y��K�؂ɒ������� 
	 * AdjustWindowRect()�͎w�肳�ꂽ�X�^�C���Ɋ�Â��āA���ۂ̃E�B���h�E�T�C�Y���w�肵���N���C�A���g�̈�����悤�ɒ�������
	 * WS_OVERLAPPEDWINDOW�́A�W���I�ȃE�B���h�E�X�^�C�� */

	// �E�B���h�E�쐬
	HWND hwnd = CreateWindowEx(
		0,
		/* 0�͊g���X�^�C���ŁA����͓��Ɋg���͍s��Ȃ��̂�0 */

		L"WindowClass",
		/* �������o�^�����E�B���h�E�N���X�̖��O */

		L"MiniDirectX11",
		/* �E�B���h�E�̃^�C�g���A��ʏ㕔�ɕ\������閼�O */

		WS_OVERLAPPEDWINDOW,
		/* WS_OVERLAPPEDWINDOW�̓E�B���h�E�X�^�C���ŁA�^�C�g���o�[��ŏ����E�ő剻�{�^�����܂܂�� */

		100,
		/* �E�B���h�E�̕\���ʒu�A��ʏ��X���W */

		100,
		/* �E�B���h�E�̕\���ʒu�A��ʏ��Y���W */

		wr.right - wr.left,
		/* �������ꂽ�� */

		wr.bottom - wr.top,
		/* �������ꂽ���� */

		nullptr,
		/* �e�E�B���h�E�A�����̂�nullptr */

		nullptr,
		/* ���j���[�A�����̂�nullptr */

		hInstance,
		/* �E�B���h�E�̃C���X�^���X�n���h�� */

		nullptr);
		/* �쐬���̒ǉ����A�����̂�nullptr */


	ShowWindow(hwnd, nCmdShow);
	/* �E�B���h�E��\�� 
	 * ShowWindow()�͍쐬�����E�B���h�E��\�����邽�߂̊֐��ŁA
	 * nCmdShow�̓E�B���h�E�̕\�����(�ʏ�̕\���A�ŏ����Ȃ�)���w�肷�� */

	// Direct3D�A�v���̃C���X�^���X�쐬
	MiniDirectX11 dxApp(hwnd);
	/* MiniDirectX11�N���X�̃C���X�^���X���쐬
	 * Direct3D���g�����`��̂��߂̃I�u�W�F�N�g���쐬���A�E�B���h�E�n���h��hwnd��n���ď��������s�� */

	MSG msg{};
	/* �E�B���h�E���b�Z�[�W�̍\���̂��`���A������
	 * Windows�̃C�x���g���b�Z�[�W���i�[���邽�߂̍\����
	 * �Ⴆ�΁A�E�B���h�E�̕���{�^���������ꂽ�E�L�[�������ꂽ�Ȃǂ��󂯎�邽�߂Ɏg�p���� */

	while (msg.message != WM_QUIT) /* ���b�Z�[�W���[�v���J�n */
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) /* ���b�Z�[�W���L���[�ɂ��邩���`�F�b�N���A����Ύ��o�� */
		{
			TranslateMessage(&msg);
			/* ���z�L�[�R�[�h�𕶎��R�[�h�ɕϊ�
			 * ���b�Z�[�W���L�[���͂̏ꍇ�A���̊֐��ŉ��z�L�[�R�[�h�𕶎��R�[�h�ɕϊ����āA
			 * ���DispatchMessage()�ŏ����ł���悤�ɂ��� */

			DispatchMessage(&msg);
			/* ���b�Z�[�W���E�B���h�E�v���V�[�W���ɑ��� 
			 * DispatchMessage()��msg��WindowProc�֐��ɑ���A�K�؂ɏ�������
			 * �Ⴆ�΃E�B���h�E�̍ĕ`���I�������Ȃ� */
		}
		else
		{
			// ���t���[���`��
			dxApp.RenderFrame();
			/* ���t���[���̕`�揈�����Ăяo��
			 * RenderFrame()��Direct3D�̕`����s���֐��ŁA�����ŉ�ʂ̓��e���X�V����
			 * ���b�Z�[�W���Ȃ��ꍇ�́A���̕`�揈�����s�����Ƃŉ�ʂ��X���[�Y�ɍX�V����� */
		}
	}

	return static_cast<int>(msg.wParam);
	/* �A�v���P�[�V�����̏I���R�[�h��Ԃ� 
	 * msg.wParam�ɂ̓A�v���P�[�V�����̏I���R�[�h���i�[����Ă���A�ʏ��0���Ԃ���� */
}

// �E�B���h�E�v���V�[�W�� �E�B���h�E�ɑ΂��郁�b�Z�[�W���������邽�߂̂���
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		/* �A�v���P�[�V�����̏I����v������
		 * PostQuitMessage��WM_QUIT���b�Z�[�W�𑗐M���A���b�Z�[�W���[�v���I������悤�Ɏw������ */
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
	/* ����̃E�B���h�E���b�Z�[�W�������s�� 
	 * �󂯎�������b�Z�[�W�����ɏ�������K�v�̂Ȃ����̂ł���΁A
	 * DefWindowProc���W���̓���A�Ⴆ�΃E�B���h�E�̈ړ���T�C�Y�ύX�Ȃǂ��s�� */
}
