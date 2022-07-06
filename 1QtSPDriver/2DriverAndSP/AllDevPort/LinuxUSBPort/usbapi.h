/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        http://github.com/signal11/hidapi .
********************************************************/

/** @file
 * @defgroup API usbapi API
 */

#ifndef USBAPI_H__
#define USBAPI_H__

#include <wchar.h>

#ifdef _WIN32
      #define USB_API_EXPORT __declspec(dllexport)
      #define USB_API_CALL
#else
      #define USB_API_EXPORT /**< API export macro */
      #define USB_API_CALL /**< API call macro */
#endif

#define USB_API_EXPORT_CALL USB_API_EXPORT USB_API_CALL /**< API export and call macro*/

#ifdef __cplusplus
extern "C" {
#endif
        struct usb_device_;
        typedef struct usb_device_ usb_device; /**< opaque hidapi structure */

		/** hidapi info structure */
        struct usb_device_info {
			/** Platform-specific device path */
			char *path;
			/** Device Vendor ID */
			unsigned short vendor_id;
			/** Device Product ID */
			unsigned short product_id;
			/** Serial Number */
			wchar_t *serial_number;
			/** Device Release Number in binary-coded decimal,
			    also known as Device Version Number */
			unsigned short release_number;
			/** Manufacturer String */
			wchar_t *manufacturer_string;
			/** Product string */
			wchar_t *product_string;
			/** Usage Page for this Device/Interface
			    (Windows/Mac only). */
			unsigned short usage_page;
			/** Usage for this Device/Interface
			    (Windows/Mac only).*/
			unsigned short usage;
			/** The USB interface which this logical device
			    represents. Valid on both Linux implementations
			    in all cases, and valid on the Windows implementation
			    only if the device contains more than one interface. */
			int interface_number;

			/** Pointer to the next device */
            struct usb_device_info *next;
		};


        /** @brief Initialize the libusb library.

            This function initializes the libusb library. Calling it is not
            strictly necessary, as it will be called automatically by
            hid_enumerate() and any of the hid_open_*() functions if it is
            needed.  This function should be called at the beginning of
            execution however, if there is a chance of libusb handles
            being opened by different threads simultaneously.

			@ingroup API

			@returns
				This function returns 0 on success and -1 on error.
		*/
        int USB_API_EXPORT USB_API_CALL usb_init(void);

        /** @brief Finalize the libusub library.

			This function frees all of the static data associated with
			HIDAPI. It should be called at the end of execution to avoid
			memory leaks.

			@ingroup API

		    @returns
				This function returns 0 on success and -1 on error.
		*/
        int USB_API_EXPORT USB_API_CALL usb_exit(void);

        /** @brief Enumerate the Vendor Specific Devices.

            This function returns a linked list of all the Vendor Specific devices
			attached to the system which match vendor_id and product_id.
			If @p vendor_id and @p product_id are both set to 0, then
            all Vendor Specific devices will be returned.

			@ingroup API
			@param vendor_id The Vendor ID (VID) of the types of device
				to open.
			@param product_id The Product ID (PID) of the types of
				device to open.

		    @returns
		    	This function returns a pointer to a linked list of type
                struct Vendor Specific_device, containing information about the Vendor Specific devices
		    	attached to the system, or NULL in the case of failure. Free
                this linked list by calling usb_free_enumeration().
		*/
        struct usb_device_info USB_API_EXPORT * USB_API_CALL usb_enumerate(unsigned short vendor_id, unsigned short product_id);

		/** @brief Free an enumeration Linked List

            This function frees a linked list created by usb_enumerate().

			@ingroup API
		    @param devs Pointer to a list of struct_device returned from
                      usb_enumerate().
		*/
        void  USB_API_EXPORT USB_API_CALL usb_free_enumeration(struct usb_device_info *devs);

        /** @brief Open a Vendor Specific Device using a Vendor ID (VID), Product ID
			(PID) and optionally a serial number.

			If @p serial_number is NULL, the first device with the
			specified VID and PID is opened.

			@ingroup API
			@param vendor_id The Vendor ID (VID) of the device to open.
			@param product_id The Product ID (PID) of the device to open.
			@param serial_number The Serial Number of the device to open
				               (Optionally NULL).

			@returns
                This function returns a pointer to a #Vendor Specific_device object on
				success or NULL on failure.
		*/
        USB_API_EXPORT usb_device * USB_API_CALL usb_open(unsigned short vendor_id, unsigned short product_id, wchar_t *serial_number);

        /** @brief Open a Vendor Specific device by its path name.

            The path name be determined by calling usb_enumerate(), or a
            platform-specific path name can be used

			@ingroup API
		    @param path The path name of the device to open

			@returns
                This function returns a pointer to a #Vendor Specific_device object on
				success or NULL on failure.
		*/
        USB_API_EXPORT usb_device * USB_API_CALL usb_open_path(const char *path);

        /** @brief Write an Output report to a Vendor Specific device.

			The first byte of @p data[] must contain the Report ID. For
			devices which only support a single report, this must be set
			to 0x0. The remaining bytes contain the report data. Since
            the Report ID is mandatory, calls to usb_write() will always
            contain one more byte than the report contains

			@ingroup API
            @param device A device handle returned from usb_open().
			@param data The data to send, including the report number as
				the first byte.
			@param length The length in bytes of the data to send.

			@returns
            @param milliseconds timeout in milliseconds or -1 for blocking wait.

            @returns
				This function returns the actual number of bytes written and
				-1 on error.
		*/
        int  USB_API_EXPORT USB_API_CALL usb_write(usb_device *device, const unsigned char *data, size_t length);

        /** @brief Read an Input report from a Vendor Specific device with timeout.

			Input reports are returned
			to the host through the INTERRUPT IN endpoint. The first byte will
			contain the Report number if the device uses numbered reports.

			@ingroup API
            @param device A device handle returned from usb_open().
			@param data A buffer to put the read data into.
			@param length The number of bytes to read. For devices with
				multiple reports, make sure to read an extra byte for
				the report number.
			@param milliseconds timeout in milliseconds or -1 for blocking wait.

			@returns
				This function returns the actual number of bytes read and
				-1 on error.
		*/
        int USB_API_EXPORT USB_API_CALL usb_read_timeout(usb_device *dev, unsigned char *data, size_t length, int milliseconds);

        /** @brief Read an Input report from a Vendor Specific device.

			Input reports are returned
		    to the host through the INTERRUPT IN endpoint. The first byte will
			contain the Report number if the device uses numbered reports.

			@ingroup API
            @param device A device handle returned from usb_open().
			@param data A buffer to put the read data into.
			@param length The number of bytes to read. For devices with
				multiple reports, make sure to read an extra byte for
				the report number.

			@returns
				This function returns the actual number of bytes read and
				-1 on error.
		*/
        int  USB_API_EXPORT USB_API_CALL usb_read(usb_device *device, unsigned char *data, size_t length);

		/** @brief Set the device handle to be non-blocking.

            In non-blocking mode calls to usb_read() will return
			immediately with a value of 0 if there is no data to be
			read. In blocking mode, hid_read() will wait (block) until
			there is data to read before returning.

			Nonblocking can be turned on and off at any time.

			@ingroup API
            @param device A device handle returned from usb_open().
			@param nonblock enable or not the nonblocking reads
			 - 1 to enable nonblocking
			 - 0 to disable nonblocking.

			@returns
				This function returns 0 on success and -1 on error.
		*/
        int  USB_API_EXPORT USB_API_CALL usb_set_nonblocking(usb_device *device, int nonblock);

		/** @brief Send a Feature report to the device.
			@ingroup API
            @param device A device handle returned from usb_open().
			@param data The data to send, including the report number as
				the first byte.
			@param length The length in bytes of the data to send, including
				the report number.

			@returns
				This function returns the actual number of bytes written and
				-1 on error.
		*/
        int USB_API_EXPORT USB_API_CALL usb_send_feature_report(usb_device *device, const unsigned char *data, size_t length);

        /** @brief Get a feature report from a Vendor Specific device.
			@ingroup API
			@returns
				This function returns the number of bytes read and
				-1 on error.
		*/
        int USB_API_EXPORT USB_API_CALL usb_get_feature_report(usb_device *device, unsigned char *data, size_t length);

        /** @brief Close a Vendor Specific device.

			@ingroup API
            @param device A device handle returned from usb_open().
		*/
        void USB_API_EXPORT USB_API_CALL usb_close(usb_device *device);

        /** @brief Get The Manufacturer String from a Vendor Specific device.

			@ingroup API
            @param device A device handle returned from usb_open().
			@param string A wide string buffer to put the data into.
			@param maxlen The length of the buffer in multiples of wchar_t.

			@returns
				This function returns 0 on success and -1 on error.
		*/
        int USB_API_EXPORT_CALL usb_get_manufacturer_string(usb_device *device, wchar_t *string, size_t maxlen);

        /** @brief Get The Product String from a Vendor Specific device.

			@ingroup API
            @param device A device handle returned from usb_open().
			@param string A wide string buffer to put the data into.
			@param maxlen The length of the buffer in multiples of wchar_t.

			@returns
				This function returns 0 on success and -1 on error.
		*/
        int USB_API_EXPORT_CALL usb_get_product_string(usb_device *device, wchar_t *string, size_t maxlen);

        /** @brief Get The Serial Number String from a Vendor Specific device.

			@ingroup API
            @param device A device handle returned from usb_open().
			@param string A wide string buffer to put the data into.
			@param maxlen The length of the buffer in multiples of wchar_t.

			@returns
				This function returns 0 on success and -1 on error.
		*/
        int USB_API_EXPORT_CALL usb_get_serial_number_string(usb_device *device, wchar_t *string, size_t maxlen);

        /** @brief Get a string from a Vendor Specific device, based on its string index.

			@ingroup API
            @param device A device handle returned from usb_open().
			@param string_index The index of the string to get.
			@param string A wide string buffer to put the data into.
			@param maxlen The length of the buffer in multiples of wchar_t.

			@returns
				This function returns 0 on success and -1 on error.
		*/
        int USB_API_EXPORT_CALL usb_get_indexed_string(usb_device *device, int string_index, wchar_t *string, size_t maxlen);

		/** @brief Get a string describing the last error which occurred.

			@ingroup API
            @param device A device handle returned from usb_open().

			@returns
				This function returns a string containing the last error
				which occurred or NULL if none has occurred.
		*/
        USB_API_EXPORT const wchar_t* USB_API_CALL usb_error(usb_device *device);

#ifdef __cplusplus
}
#endif

#endif

