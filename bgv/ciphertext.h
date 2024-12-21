#pragma once

#include "context.h"
#include "encryptionparams.h"
#include <Eigen/Dense>

namespace fheprac
{
	class Ciphertext
	{
	public:
		Ciphertext();

		Ciphertext(uint64_t poly_count, uint64_t slot_count);

		int64_t& operator()(uint64_t poly_index, uint64_t slot_index);

		EncryptionParameters& param();

		void assign(uint64_t poly_count, uint64_t slot_count);

		uint64_t poly_count() const;

		uint64_t slot_count() const;

	private:
		uint64_t poly_count_;

		uint64_t slot_count_;

		std::vector<Eigen::Vector<int64_t, Eigen::Dynamic>> polys_;

		EncryptionParameters param_;
	};
}