#include "ciphertext.h"

namespace fheprac
{
	Ciphertext::Ciphertext(PolyMatrix poly_matrix) : ct_(poly_matrix), param_(EncryptionParameters()) {}

	PolyMatrix& Ciphertext::data()
	{
		return ct_;
	}

	EncryptionParameters& Ciphertext::param()
	{
		return param_;
	}
}