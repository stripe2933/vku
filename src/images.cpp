#include <vku/images.hpp>

vku::AllocatedImage::AllocatedImage(
    vma::Allocator _allocator,
    const vk::ImageCreateInfo &createInfo,
    const vma::AllocationCreateInfo &allocationCreateInfo
): Image { nullptr, createInfo.extent, createInfo.format, createInfo.mipLevels, createInfo.arrayLayers },
   allocator { _allocator } {
    std::tie(image, allocation) = allocator.createImage(createInfo, allocationCreateInfo);
}

vku::AllocatedImage::AllocatedImage(
    AllocatedImage &&src
) noexcept : Image { static_cast<Image>(src) },
             allocator { src.allocator },
             allocation { std::exchange(src.allocation, nullptr) } {
}

auto vku::AllocatedImage::operator=(
    AllocatedImage &&src
) noexcept -> AllocatedImage & {
    if (allocation) {
        allocator.destroyImage(image, allocation);
    }

    static_cast<Image &>(*this) = static_cast<Image>(src);
    allocator = src.allocator;
    allocation = std::exchange(src.allocation, nullptr);
    return *this;
}

vku::AllocatedImage::~AllocatedImage() {
    if (allocation) {
        allocator.destroyImage(image, allocation);
    }
}