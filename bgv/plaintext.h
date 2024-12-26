#pragma once

#include "context.h"
#include "util/polymatrix.h"

namespace fheprac
{
	class Plaintext
	{
	public:
		Plaintext() = default;

		Plaintext(PolyMatrix poly_matrix);

		PolyMatrix& data();

	private:
		PolyMatrix pt_;
	};
}