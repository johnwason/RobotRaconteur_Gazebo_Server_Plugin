/*
 * Copyright (C) 2016 Wason Technology, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include "CameraImpl.h"
#include "robotraconteur_gazebo_server_plugin.h"
#include <gazebo/rendering/rendering.hh>

namespace RobotRaconteurGazeboServerPlugin
{
	CameraImpl::CameraImpl(sensors::CameraSensorPtr gz_camera) : SensorImpl(gz_camera)
	{

	}

	void CameraImpl::Init()
	{
		RR_WEAK_PTR<SensorImpl> c=shared_from_this();
		updateConnection=get_camera()->ConnectUpdated(boost::bind(&CameraImpl::OnUpdate,c));
	}

	rrgz::CameraImagePtr CameraImpl::CaptureImage()
	{

		sensors::CameraSensorPtr c=get_camera();
		rendering::CameraPtr c2=c->Camera();
		if (!c2->CaptureData()) throw std::runtime_error("Image not ready");
		rrgz::CameraImagePtr o(new rrgz::CameraImage());
		const uint8_t* image_bytes=c2->ImageData();
		size_t image_byte_size=c2->ImageByteSize();
		o->data=RR::AttachRRArrayCopy(image_bytes,image_byte_size);
		o->format=c2->ImageFormat();
		o->width=c2->ImageWidth();
		o->height=c2->ImageHeight();

		return o;
	}

	sensors::CameraSensorPtr CameraImpl::get_camera()
	{
		return std::dynamic_pointer_cast<sensors::CameraSensor>(get_sensor());
	}

	RR::PipePtr<rrgz::CameraImagePtr> CameraImpl::get_ImageStream()
	{
		boost::mutex::scoped_lock lock(this_lock);
		return m_ImageStream;
	}
	void CameraImpl::set_ImageStream(RR::PipePtr<rrgz::CameraImagePtr> value)
	{
		boost::mutex::scoped_lock lock(this_lock);
		if (m_ImageStream) throw std::runtime_error("Already set");
		m_ImageStream=value;
		m_ImageStream_b=RR_MAKE_SHARED<RR::PipeBroadcaster<rrgz::CameraImagePtr> >();
		m_ImageStream_b->Init(m_ImageStream,3);
	}

	void CameraImpl::OnUpdate(RR_WEAK_PTR<SensorImpl> c)
	{
		RR_SHARED_PTR<CameraImpl> c1=RR_DYNAMIC_POINTER_CAST<CameraImpl>(c.lock());
		if (!c1) return;
		c1->OnUpdate1();
	}

	void CameraImpl::OnUpdate1()
	{

		RR::PipeBroadcasterPtr<rrgz::CameraImagePtr> b;
		{
		boost::mutex::scoped_lock lock(this_lock);
		b=m_ImageStream_b;
		}
		if (b)
		{

			auto i=CaptureImage();
			b->AsyncSendPacket(i, []() {});
		}

	}
}