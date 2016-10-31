var golobal_template_json_obj = null;

//加载外设模板
function get_peripherals_template(template) {
	try {
		var test_template = JSON.stringify(template);
	} catch(e) {
		alert("数据模板错误");
	}

	//alert(JSON.stringify(template));

	var html = template_to_html(template);
	if(html == false) {
		return;
	}

	//	alert(html);
	if(html != "") {
		document.getElementById("template").innerHTML = html;
		document.getElementById("template").style.display = "block";
	} else {
		document.getElementById("template").style.display = "none";
	}

	golobal_template_json_obj = template; //更新全局模板

	listen_switch();

	mui('.mui-switch')['switch']();
}

//解析模板,将其转为html文件
function template_to_html(template) {
	var html_total = "";
	var read_html = "";
	var bool_ctrl_html = "";
	var pwm_ctrl_html = "";
	var private_html = "";
	
	var node_html = "";

	try {
		//1.解析template.command_id和template.CMD,并做处理
		var command_id = template.command_id;
		var CMD = template.CMD;

		//alert(command_id + "   " + CMD);

		//2.将template.G数组中得成员,按照template.G[i].T来排序
		var template_G = template.G;
		template_G.sort(
			function my_sort(a, b) {
				return a.T - b.T;
			}
		);

		//alert(JSON.stringify(template_G));

		//3.解析template_G中的内容
		for(var index in template_G) {

			if((typeof(template_G[index].T) == "undefined") || (typeof(template_G[index].N) == "undefined") || (typeof(template_G[index].PID) == "undefined") || (typeof(template_G[index].E) == "undefined")) {
				alert("数据模板错误,请检查模板, PID = " + template_G[index].PID);
				return false;
			}

			var type = template_G[index].T;
			//最大值和最小值过滤一次
			if(type < _TYPE_READ_MIN || type > _TYPE_PRIVATE_MAX) {
				alert("type超过范围,请检查模板, PID = " + template_G[index].PID);
				return false;
			}

			//数据分类
			if((type >= _TYPE_READ_MIN) && (type <= _TYPE_READ_MAX)) {

				if((typeof(template_G[index].U) == "undefined")) {
					alert("缺少单位,请检查模板, PID = " + template_G[index].PID);
					return false;
				}
				
				node_html = type_read_to_html(template_G[index]);
				if(node_html == false) {
					alert("解析模板错误,请检查代码,T:" + type);
					return false;
				}
				read_html += node_html;
			} else if(type >= _TYPE_BOOL_CTRL_MIN && type <= _TYPE_BOOL_CTRL_MAX) {

				node_html = type_bool_ctrl_to_html(template_G[index]);
				if(node_html == false) {
					alert("模板错误,请检查代码,T:" + type);
					return false;
				}
				
				bool_ctrl_html += node_html;
			} else if(type >= _TYPE_PWM_CTRL_MIN && type <= _TYPE_PWM_CTRL_MAX) {
				if((typeof(template_G[index].D_MIN) == "undefined") || (typeof(template_G[index].D_MAX) == "undefined")) {
					alert("最大值最小值错误,请检查模板, PID = " + template_G[index].PID);
					return false;
				}
				
				node_html = type_pwm_ctrl_to_html(template_G[index]);
				if(node_html == false) {
					alert("模板错误,请检查代码,T:" + type);
					return false;
				}
				pwm_ctrl_html += node_html;
			} else if(type >= _TYPE_PRIVATE_MIN && type <= _TYPE_PRIVATE_MAX) {
				node_html = type_private_to_html(template_G[index]);
				if(node_html == false) {
					alert("模板错误,请检查代码,T:" + type);
					return false;
				}
				private_html += node_html;
			}
		}

		read_html = '<div><ul class="mui-table-view peripherals_ul_style">' + read_html + '</ul></div>';
		bool_ctrl_html = '<div><ul class="mui-table-view peripherals_ul_style">' + bool_ctrl_html + '</ul></div>';
		pwm_ctrl_html = '<div><ul class="mui-table-view peripherals_ul_style">' + pwm_ctrl_html + '</ul></div>';
		private_html = '<div><ul class="mui-table-view peripherals_ul_style">' + private_html + '</ul></div>';

		html_total += read_html + bool_ctrl_html + pwm_ctrl_html + private_html;
		return html_total;
	} catch(e) {
		alert(e.message);
		return false;
	}

}

//处理只读数据类  将其转化为对应的html
function type_read_to_html(read_data) {
	try {
		var pid = read_data.PID; //PID
		var type = read_data.T; //类型
		var name = read_data.N; //名称
		var extern = read_data.E; //扩展信息
		var unit = read_data.U; //单位

		var li_html = "";
		var icon_div = "";
		var name_div = "";
		var unit_div = "";
		var data_div = "";

		icon_div = '<div class="peripherals_li_icon"><label><span class="' + type_to_icon_table[type] + '"></span></label></div>';
		name_div = '<div class="peripherals_li_name">' + name + '</div>';

		if(unit != "") {
			unit_div = '<div class="peripherals_li_unit"><span class="mui-badge mui-badge-warning">' + unit + '</span></div>';
		}

		data_div = '<div class="peripherals_li_data"><span id="' + pid + '_' + type + '">0</span></div>';

		li_html = '<li class="mui-table-view-cell">' + icon_div + name_div + unit_div + data_div + '</li>';
		
		return li_html;
	} catch(e) {
		alert("type_read_to_html error + message:" + e.message);
		
		return false;
	}
}

//处理bool控制类外设  将其转化为对应的html
function type_bool_ctrl_to_html(read_data) {
	try {
		var pid = read_data.PID; //PID
		var type = read_data.T; //类型
		var name = read_data.N; //名称
		var extern = read_data.E; //扩展信息

		var li_html = "";
		var icon_div = "";
		var name_div = "";
		var switch_div = "";

		icon_div = '<div class="peripherals_li_icon"><label><span class="' + type_to_icon_table[type] + '"></span></label></div>';
		name_div = '<div class="peripherals_li_name">' + name + '</div>';
		switch_div = '<div id="' + pid + '_' + type + '" class="mui-switch mui-switch-blue peripherals_li_single_div_right mico_peripherals_switch"><div class="mui-switch-handle"></div></div>';

		li_html = '<li class="mui-table-view-cell">' + icon_div + name_div + switch_div + '</li>';

		//alert(li_html);

		return li_html;
	} catch(e) {
		alert("type_bool_ctrl_to_html error + message:" + e.message)
		return false;
	}

}

//处理pwm控制类外设  将其转化为对应的html
function type_pwm_ctrl_to_html(read_data) {
	try {
		var pid = read_data.PID; //PID
		var type = read_data.T; //类型
		var name = read_data.N; //名称
		var extern = read_data.E; //扩展信息
		var data_min = read_data.D_MIN; //最小值
		var data_max = read_data.D_MAX; //最小值

		var li_id = pid + '_' + type + '_' + data_min + '_' + data_max;

		var li_html = "";
		var icon_div = "";
		var name_div = "";
		var right_div = "";

		if((typeof(data_min) != 'number') || (typeof(data_max) != 'number') || (data_min > data_max)) {
			alert('最大值最小值错误,' + pid);
			return false;
		}

		icon_div = '<div class="peripherals_li_icon"><label><span class="' + type_to_icon_table[type] + '"></span></label></div>';
		name_div = '<div class="peripherals_li_name">' + name + '</div>';
		right_div = '<div class="peripherals_li_single_div_right"><span class="mui-icon mui-icon-arrowright"></span></div>';

		li_html = '<li class="mui-table-view-cell" id="' + li_id + '" onclick="goto_pwm_ctrl_html(this)">' + icon_div + name_div + right_div + '</li>';

		//alert(li_html);

		return li_html;
	} catch(e) {
		alert("type_pwm_ctrl_to_html error,message" + e.message);
		return false;
	}
}

//处理私有控制类控制类外设  将其转化为对应的html
function type_private_to_html(read_data) {
	try {
		var pid = read_data.PID; //PID
		var type = read_data.T; //类型
		var name = read_data.N; //名称
		var extern = read_data.E; //扩展信息
		var li_id = pid;

		var li_html = "";
		var icon_div = "";
		var name_div = "";
		var right_div = "";

		icon_div = '<div class="peripherals_li_icon"><label><span class="' + type_to_icon_table[type] + '"></span></label></div>';
		name_div = '<div class="peripherals_li_name">' + name + '</div>';
		right_div = '<div class="peripherals_li_single_div_right"><span class="mui-icon mui-icon-arrowright"></span></div>';

		li_html = '<li class="mui-table-view-cell" id="' + li_id + '_' + type + '" onclick="goto_private_html(this)">' + icon_div + name_div + right_div + '</li>';

		//alert(li_html);

		return li_html;
	} catch(e) {
		alert("type_private_to_html error,message" + e.message);
		return false;
	}
}