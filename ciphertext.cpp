#include "ciphertext.h"

namespace fheprac
{
	Ciphertext::Ciphertext() :poly_count_(0), slot_count_(0) {}

	Ciphertext::Ciphertext(uint64_t poly_count, uint64_t slot_count) :poly_count_(poly_count), slot_count_(slot_count)
	{
		polys_.assign(poly_count_, Eigen::Vector<int64_t, Eigen::Dynamic>(slot_count_));
	}

	int64_t& Ciphertext::operator()(uint64_t poly_index, uint64_t slot_index)
	{
		// TODO: ∫§≈Õ ¿Œµ¶Ω∫ øπø‹√≥∏Æ
		return polys_[poly_index][slot_index];
	}

	EncryptionParameters& Ciphertext::param()
	{
		return param_;
	}

	void Ciphertext::assign(uint64_t poly_count, uint64_t slot_count)
	{
		poly_count_ = poly_count;
		slot_count_ = slot_count;
		polys_.assign(poly_count_, Eigen::Vector<int64_t, Eigen::Dynamic>(slot_count_));
	}

	uint64_t Ciphertext::poly_count() const
	{
		return poly_count_;
	}

	uint64_t Ciphertext::slot_count() const
	{
		return slot_count_;
	}
}