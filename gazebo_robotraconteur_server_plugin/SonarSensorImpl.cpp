/*
 * Copyright (C) 2016-2020 Wason Technology, LLC
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
	SonarSensorImpl::SonarSensorImpl(sensors::SonarSensorPtr gz_sonar) : SensorImpl(gz_sonar)
	{
		this->gz_sonar=gz_sonar;
	}

	void SonarSensorImpl::RRServiceObjectInit(RR_WEAK_PTR<RR::ServerContext> context, const std::string& service_path)
	{
		
		SensorImpl::RRServiceObjectInit(context, service_path);

		rr_downsampler->AddWireBroadcaster(rrvar_range);

		boost::weak_ptr<SonarSensorImpl> weak_this = RR::rr_cast<SonarSensorImpl>(shared_from_this());
		this->rrvar_range->GetWire()->SetPeekInValueCallback(
			[weak_this](uint32_t ep) {
				auto this_ = weak_this.lock();
				if (!this_) throw RR::InvalidOperationException("Entity has been released");
				return this_->get_sonarsensor()->Range();
			}
		);
	}

	
	double SonarSensorImpl::get_range_min()
	{
		return get_sonarsensor()->RangeMin();
	}
	
	double SonarSensorImpl::get_range_max()
	{
		return get_sonarsensor()->RangeMax();
	}
	
	double SonarSensorImpl::get_radius()
	{
		return get_sonarsensor()->Radius();
	}
	
	sensors::SonarSensorPtr SonarSensorImpl::get_sonarsensor()
	{
		return std::dynamic_pointer_cast<sensors::SonarSensor>(get_sensor());
	}

	void SonarSensorImpl::OnUpdate1()
	{		
		SensorImpl::OnUpdate1();
		
		auto s = gz_sonar.lock();
		if (!s) return;

		auto i = s->Range();
		rrvar_range->SetOutValue(i);		
	}

}
