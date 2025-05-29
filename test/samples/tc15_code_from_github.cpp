/**************************************************************************/
/*  buffer_decoder.h                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/io/image.h"
#include "core/templates/vector.h"

class CameraFeed;

struct StreamingBuffer {
	void *start = nullptr;
	size_t length = 0;
};

class BufferDecoder {
protected:
	CameraFeed *camera_feed = nullptr;
	Ref<Image> image;
	int width = 0;
	int height = 0;

public:
	virtual void decode(StreamingBuffer p_buffer) = 0;

	BufferDecoder(CameraFeed *p_camera_feed);
	virtual ~BufferDecoder() {}
};

class AbstractYuyvBufferDecoder : public BufferDecoder {
protected:
	int *component_indexes = nullptr;

public:
	AbstractYuyvBufferDecoder(CameraFeed *p_camera_feed);
	~AbstractYuyvBufferDecoder();
};

class SeparateYuyvBufferDecoder : public AbstractYuyvBufferDecoder {
private:
	Vector<uint8_t> y_image_data;
	Vector<uint8_t> cbcr_image_data;
	Ref<Image> y_image;
	Ref<Image> cbcr_image;

public:
	SeparateYuyvBufferDecoder(CameraFeed *p_camera_feed);
	virtual void decode(StreamingBuffer p_buffer) override;
};

class YuyvToGrayscaleBufferDecoder : public AbstractYuyvBufferDecoder {
private:
	Vector<uint8_t> image_data;

public:
	YuyvToGrayscaleBufferDecoder(CameraFeed *p_camera_feed);
	virtual void decode(StreamingBuffer p_buffer) override;
};

class YuyvToRgbBufferDecoder : public AbstractYuyvBufferDecoder {
private:
	Vector<uint8_t> image_data;

public:
	YuyvToRgbBufferDecoder(CameraFeed *p_camera_feed);
	virtual void decode(StreamingBuffer p_buffer) override;
};

class CopyBufferDecoder : public BufferDecoder {
private:
	Vector<uint8_t> image_data;
	bool rgba = false;

public:
	CopyBufferDecoder(CameraFeed *p_camera_feed, bool p_rgba);
	virtual void decode(StreamingBuffer p_buffer) override;
};

class JpegBufferDecoder : public BufferDecoder {
private:
	Vector<uint8_t> image_data;

public:
	JpegBufferDecoder(CameraFeed *p_camera_feed);
	virtual void decode(StreamingBuffer p_buffer) override;
};


class CameraServer : public Object {
	GDCLASS(CameraServer, Object);
	_THREAD_SAFE_CLASS_

public:
	enum FeedImage {
		FEED_RGBA_IMAGE = 0,
		FEED_YCBCR_IMAGE = 0,
		FEED_Y_IMAGE = 0,
		FEED_CBCR_IMAGE = 1,
		FEED_IMAGES = 2
	};

	typedef CameraServer *(*CreateFunc)();

private:
protected:
	static CreateFunc create_func;

	bool monitoring_feeds = false;
	Vector<Ref<CameraFeed>> feeds;

	static CameraServer *singleton;

	static void _bind_methods();

	template <typename T>
	static CameraServer *_create_builtin() {
		return memnew(T);
	}

public:
	static CameraServer *get_singleton();

	template <typename T>
	static void make_default() {
		create_func = _create_builtin<T>;
	}

	static CameraServer *create() {
		CameraServer *server = create_func ? create_func() : memnew(CameraServer);
		return server;
	}

	virtual void set_monitoring_feeds(bool p_monitoring_feeds);
	_FORCE_INLINE_ bool is_monitoring_feeds() const { return monitoring_feeds; }

	// Right now we identify our feed by it's ID when it's used in the background.
	// May see if we can change this to purely relying on CameraFeed objects or by name.
	int get_free_id();
	int get_feed_index(int p_id);
	Ref<CameraFeed> get_feed_by_id(int p_id);

	// Add and remove feeds.
	void add_feed(const Ref<CameraFeed> &p_feed);
	void remove_feed(const Ref<CameraFeed> &p_feed);

	// Get our feeds.
	Ref<CameraFeed> get_feed(int p_index);
	int get_feed_count();
	TypedArray<CameraFeed> get_feeds();

	// Intended for use with custom CameraServer implementation.
	RID feed_texture(int p_id, FeedImage p_texture);

	CameraServer();
	~CameraServer();
};

VARIANT_ENUM_CAST(CameraServer::FeedImage);


class CameraFeed : public RefCounted {
	GDCLASS(CameraFeed, RefCounted);

public:
	enum FeedDataType {
		FEED_NOIMAGE, // we don't have an image yet
		FEED_RGB, // our texture will contain a normal RGB texture that can be used directly
		FEED_YCBCR, // our texture will contain a YCbCr texture that needs to be converted to RGB before output
		FEED_YCBCR_SEP, // our camera is split into two textures, first plane contains Y data, second plane contains CbCr data
		FEED_EXTERNAL, // specific for android atm, camera feed is managed externally, assumed RGB for now
	};

	enum FeedPosition {
		FEED_UNSPECIFIED, // we have no idea
		FEED_FRONT, // this is a camera on the front of the device
		FEED_BACK // this is a camera on the back of the device
	};

private:
	int id; // unique id for this, for internal use in case feeds are removed

protected:
	struct FeedFormat {
		int width = 0;
		int height = 0;
		String format;
		int frame_numerator = 0;
		int frame_denominator = 0;
		uint32_t pixel_format = 0;
	};

	String name; // name of our camera feed
	FeedDataType datatype; // type of texture data stored
	FeedPosition position; // position of camera on the device
	Transform2D transform; // display transform
	int base_width = 0;
	int base_height = 0;
	Vector<FeedFormat> formats;
	Dictionary parameters;
	int selected_format = -1;

	bool active; // only when active do we actually update the camera texture each frame
	RID texture[CameraServer::FEED_IMAGES]; // texture images needed for this

	static void _bind_methods();

public:
	int get_id() const;
	bool is_active() const;
	void set_active(bool p_is_active);

	String get_name() const;
	void set_name(String p_name);

	int get_base_width() const;
	int get_base_height() const;

	FeedPosition get_position() const;
	void set_position(FeedPosition p_position);

	Transform2D get_transform() const;
	void set_transform(const Transform2D &p_transform);

	RID get_texture(CameraServer::FeedImage p_which);
	uint64_t get_texture_tex_id(CameraServer::FeedImage p_which);

	CameraFeed();
	CameraFeed(String p_name, FeedPosition p_position = CameraFeed::FEED_UNSPECIFIED);
	virtual ~CameraFeed();

	FeedDataType get_datatype() const;
	void set_rgb_image(const Ref<Image> &p_rgb_img);
	void set_ycbcr_image(const Ref<Image> &p_ycbcr_img);
	void set_ycbcr_images(const Ref<Image> &p_y_img, const Ref<Image> &p_cbcr_img);
	void set_external(int p_width, int p_height);

	virtual bool set_format(int p_index, const Dictionary &p_parameters);
	virtual Array get_formats() const;
	virtual FeedFormat get_format() const;

	virtual bool activate_feed();
	virtual void deactivate_feed();

	GDVIRTUAL0R(bool, _activate_feed)
	GDVIRTUAL0(_deactivate_feed)
};

VARIANT_ENUM_CAST(CameraFeed::FeedDataType);
VARIANT_ENUM_CAST(CameraFeed::FeedPosition);


class CameraMacOS : public CameraServer {
	GDSOFTCLASS(CameraMacOS, CameraServer);

public:
	CameraMacOS() = default;

	void update_feeds();
	void set_monitoring_feeds(bool p_monitoring_feeds) override;
};


class CameraFeedLinux : public CameraFeed {
	GDSOFTCLASS(CameraFeedLinux, CameraFeed);

private:
	SafeFlag exit_flag;
	Thread *thread = nullptr;
	String device_name;
	int file_descriptor = -1;
	StreamingBuffer *buffers = nullptr;
	unsigned int buffer_count = 0;
	BufferDecoder *buffer_decoder = nullptr;

	static void update_buffer_thread_func(void *p_func);

	void _update_buffer();
	void _query_device(const String &p_device_name);
	void _add_format(v4l2_fmtdesc description, v4l2_frmsize_discrete size, int frame_numerator, int frame_denominator);
	bool _request_buffers();
	bool _start_capturing();
	void _read_frame();
	void _stop_capturing();
	void _unmap_buffers(unsigned int p_count);
	BufferDecoder *_create_buffer_decoder();
	void _start_thread();

public:
	String get_device_name() const;
	bool activate_feed() override;
	void deactivate_feed() override;
	bool set_format(int p_index, const Dictionary &p_parameters) override;
	Array get_formats() const override;
	FeedFormat get_format() const override;

	CameraFeedLinux(const String &p_device_name);
	~CameraFeedLinux() override;
};