$(call inherit-product-if-exists, target/vego/arbutus-common/arbutus-common.mk)

PRODUCT_PACKAGES +=

PRODUCT_COPY_FILES +=

PRODUCT_AAPT_CONFIG := large xlarge hdpi xhdpi
PRODUCT_AAPT_PERF_CONFIG := xhdpi
PRODUCT_CHARACTERISTICS := musicbox

PRODUCT_BRAND := vego
PRODUCT_NAME := arbutus_faceid
PRODUCT_DEVICE := arbutus-faceid
PRODUCT_MODEL := arbutus faceid
