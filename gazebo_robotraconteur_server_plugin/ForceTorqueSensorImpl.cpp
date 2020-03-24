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

#include "robotraconteur_gazebo_server_plugin.h"
#include <gazebo/rendering/rendering.hh>

namespace RobotRaconteurGazeboServerPlugin
{
	ForceTorqueSensorImpl::ForceTorqueSensorImpl(sensors::ForceTorqueSensorPtr gz_contact) : SensorImpl(gz_contact)
	{

	}

	void ForceTorqueSensorImpl::Init()
	{
		RR_WEAK_PTR<SensorImpl> c=shared_from_this();
		updateConnection=get_forcetorquesensor()->ConnectUpdated(boost::bind(&ForceTorqueSensorImpl::OnUpdate,c));
	}

	void ForceTorqueSensorImpl::OnUpdate(RR_WEAK_PTR<SensorImpl> c)
	{
		RR_SHARED_PTR<ForceTorqueSensorImpl> c1=RR_DYNAMIC_POINTER_CAST<ForceTorqueSensorImpl>(c.lock());
		if (!c1) return;
		c1->OnUpdate1();
	}


	static geometry::Wrench gz_forcetorque_to_rr_wrench(const sensors::ForceTorqueSensorPtr& ft)
	{
		auto t = ft->Torque();
		auto f = ft->Force();

		geometry::Wrench o;
		o.s.force.s.x = t.X();
		o.s.force.s.y = t.Y();
		o.s.force.s.z = t.Z();
		o.s.torque.s.x = t.X();
		o.s.torque.s.y = t.Y();
		o.s.torque.s.z = t.Z();
		return o;
	}

	void ForceTorqueSensorImpl::set_ForceTorque(RR::WirePtr<geometry::Wrench> value)
	{
		ForceTorqueSensor_default_abstract_impl::set_ForceTorque(value);
		boost::weak_ptr<ForceTorqueSensorImpl> weak_this = RR::rr_cast<ForceTorqueSensorImpl>(shared_from_this());
		this->rrvar_ForceTorque->GetWire()->SetPeekInValueCallback(
			[weak_this](uint32_t ep) {
				auto this_ = weak_this.lock();
				if (!this_) throw RR::InvalidOperationException("Entity has been released");
				return gz_forcetorque_to_rr_wrench(this_->get_forcetorquesensor());
			}
		);
	}

	sensors::ForceTorqueSensorPtr ForceTorqueSensorImpl::get_forcetorquesensor()
	{
		return std::dynamic_pointer_cast<sensors::ForceTorqueSensor>(get_sensor());
	}

	void ForceTorqueSensorImpl::OnUpdate1()
	{
		RR::WireBroadcasterPtr<geometry::Wrench> b;
		{
		boost::mutex::scoped_lock lock(ForceTorqueSensor_default_abstract_impl::this_lock);
		b=rrvar_ForceTorque;
		}
		if (b)
		{
			auto i=get_ForceTorque();
			b->SetOutValue(gz_forcetorque_to_rr_wrench(get_forcetorquesensor()));
		}
	}

}
