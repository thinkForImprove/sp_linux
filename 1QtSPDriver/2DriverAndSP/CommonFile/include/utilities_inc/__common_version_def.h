#ifndef _COMMON_VERSION_DEF_H_
#define _COMMON_VERSION_DEF_H_


#define TYPE_STATIC					"static"
#define TYPE_DYNAMIC					"dynamic"
#define TYPE_EXECUTABLE					"executable"


// define version functions
#define STATIC_VERSION_FUNCTIONS(ProductName, ProductVersion, ProductType) \
	static const char* get_product_name() { return (ProductName); }\
	static const char* get_product_version() { return (ProductVersion); }\
	static const char* get_product_type() { return (ProductType); }

#define DEFINE_STATIC_VERSION_FUNCTIONS(ProductName, ProductVersion, ProductType) \
public:\
	STATIC_VERSION_FUNCTIONS(ProductName, ProductVersion, ProductType)


#endif // _COMMON_VERSION_DEF_H_
