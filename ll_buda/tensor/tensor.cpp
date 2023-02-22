#include "ll_buda/tensor/tensor.hpp"

#include "ll_buda/tensor/tensor_impl.hpp"
#include "ll_buda/tensor/tensor_impl_wrapper.hpp"
#include "common/bfloat16.hpp"
#include "llrt/llrt.hpp"
#include "constants.hpp"

using namespace tt::constants;

namespace tt {

namespace ll_buda {

Tensor::Tensor(const std::array<uint32_t, 4> &shape, Initialize init_type, DataType data_type, Layout layout)
    : shape_(shape), strides_(compute_strides()), data_type_(data_type), layout_(layout) {
    tensor_impl::initialize_data_wrapper(*this, init_type);
}

Tensor::Tensor(std::vector<bfloat16> data, const std::array<uint32_t, 4> &shape, DataType data_type, Layout layout, Device *device)
    : shape_(shape), strides_(compute_strides()), data_type_(data_type), layout_(layout), device_(device) {
    tensor_impl::initialize_data_on_device<bfloat16>(*this, data);
}

Tensor::Tensor(std::vector<uint32_t> data, const std::array<uint32_t, 4> &shape, DataType data_type, Layout layout, Device *device)
    : shape_(shape), strides_(compute_strides()), data_type_(data_type), layout_(layout), device_(device) {
    tensor_impl::initialize_data_on_device<uint32_t>(*this, data);
}

Tensor::Tensor(std::vector<float> data, const std::array<uint32_t, 4> &shape, DataType data_type, Layout layout, Device *device)
    : shape_(shape), strides_(compute_strides()), data_type_(data_type), layout_(layout), device_(device) {
    tensor_impl::initialize_data_on_device<float>(*this, data);
}

Tensor::Tensor(const std::array<uint32_t, 4> &shape, Initialize init_type, DataType data_type, Layout layout, Device *device)
    : shape_(shape), strides_(compute_strides()), data_type_(data_type), layout_(layout), device_(device) {
    tensor_impl::initialize_data_wrapper(*this, init_type);
}

Tensor::Tensor(const std::array<uint32_t, 4> &shape, DataType data_type, Layout layout, Device *device)
    : shape_(shape), strides_(compute_strides()), data_type_(data_type), layout_(layout), device_(device) {
    TT_ASSERT(device != nullptr);
    TT_ASSERT(layout == Layout::ROW_MAJOR or layout == Layout::TILE, "Only ROW_MAJOR or TILE layout is supported!");
    uint32_t packed_size_in_bytes = tensor_impl::packed_buffer_size_bytes_wrapper(data_type, volume());
    tensor_impl::allocate_interleaved_buffer_on_device(*this, packed_size_in_bytes);
}

Tensor Tensor::to(Device *target_device) const {
    if (on_host()) {
        return tensor_impl::to_device_wrapper(*this, target_device);
    }
    TT_ASSERT(this->device_ == target_device && "Currently do not support moving between devices");
    return Tensor(*this);
}

Tensor Tensor::to(Host *host) const {
    if (on_host()) {
        return Tensor(*this);
    }
    return tensor_impl::to_host_wrapper(*this);
}

void Tensor::print(Layout print_layout, bool pretty_print) const {
    tensor_impl::print_wrapper(*this, print_layout, pretty_print);
}

// Prints like numpy print function to make it more readable. Only supports row major layout.
void Tensor::pretty_print(Layout print_layout) const {
    print(print_layout, /*pretty_print=*/true);
}

const std::array<uint32_t, 4>& Tensor::reshape(int N, int C, int H, int W) {
    vector<int> ns{N, C, H, W};
    int neg_idx = -1;
    for (int i = 0; i < ns.size(); i++) {
        if (ns[i] == -1) {
            TT_ASSERT(neg_idx == -1, "Only one -1 is allowed in Tensor::reshape");
            neg_idx = i;
        } else {
            TT_ASSERT(ns[i] > 0, "New shape entries can only have -1 or positive values");
        }
    }

    uint32_t old_volume = this->volume();

    switch (neg_idx) {
        case 0:
            TT_ASSERT(old_volume % C*H*W == 0);
            N = old_volume/(C*H*W);
            break;
        case 1:
            TT_ASSERT(old_volume % N*H*W == 0);
            C = old_volume/(N*H*W);
            break;
        case 2:
            TT_ASSERT(old_volume % N*C*W == 0);
            H = old_volume/(N*C*W);
            TT_ASSERT(H%32 == 0);
            break;
        case 3:
            TT_ASSERT(old_volume % N*C*H == 0);
            W = old_volume/(N*C*H);
            TT_ASSERT(W%32 == 0);
            break;
        case -1: // In case where there is no negative value in ns
            TT_ASSERT(N*C*H*W == old_volume);
            break;
        default:
            TT_ASSERT(false && "Unexpected neg_idx in Tensor::reshape!");
    }

    if (this->layout() == Layout::TILE) {
        TT_ASSERT(H % 32 == 0 && W % 32 == 0 && "Expected a multiple of 32 for H, W (or -1 evaluating to such) in Tensor::reshape()!");
    }

    shape_[0] = N;
    shape_[1] = C;
    shape_[2] = H;
    shape_[3] = W;
    strides_ = compute_strides();

    return shape_;
}

}  // namespace ll_buda

}  // namespace tt
