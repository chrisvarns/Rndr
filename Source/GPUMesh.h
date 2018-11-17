#pragma once

struct ID3D11Buffer;

struct GPUMesh {
	ID3D11Buffer*	positionBuffer;
	ID3D11Buffer*	indexBuffer;
	ID3D11Buffer*	normalBuffer;
	ID3D11Buffer*	uvBuffer;
};