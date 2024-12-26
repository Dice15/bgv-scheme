#pragma once

#include "context.h"
#include "encryptionparams.h"
#include "util/polymatrix.h"

namespace fheprac
{
	class Ciphertext
	{
	public:
		Ciphertext() = default;

		Ciphertext(PolyMatrix poly_matrix);

		PolyMatrix& data();

		EncryptionParameters& param();

	private:
		PolyMatrix ct_;

		EncryptionParameters param_;
	};
}