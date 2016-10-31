var member_device_id = '';

function quit_management_page(){
	send_event('refesh_device_list', 'device_management');
	closeWin();
}

//获取设备的app成员列表
function get_member_list() {
	var app_token = check_app_token();
	
	if(monitor_net() == "none") {
		quit_management_page();
		return;
	}

	var param = {
		deviceid: member_device_id,
		token: app_token
	};

	//alert(JSON.stringify(param));

	mico2.getMemberList(param, function(ret, err) {
		try {
			if(ret && ret.meta.code == 0) {

				if(ret.data == null || ret.data == 'undefined') {
					mui.toast("获取成员列表失败");
					return;
				}

				process_member_list(ret.data);
			} else {
				alert("获取授权信息失败,err: " + JSON.stringify(err))
			}

		} catch(e) {
			mui.toast("获取授权信息失败,请重试!");
		}
	});
	}

//动态生成授权的成员列表
function process_member_list(data) {
	var i = 0;
	var icon_html = "";
	var slider_right_html = "";
	var text_html = "";
	var slider_handle_html = "";
	var li_html = "";
	var total_html = "";
	var li_total = "";
	
	for(i = 0; i < data.length; i++) {
		//alert(JSON.stringify(data[i]));

		var enduserid = data[i].enduserid;
		var app = data[i].app;
		var phone = data[i].phone;
		var role = data[i].role;
		var email = data[i].email;

		icon_html = "";
		slider_right_html = "";
		text_html = "";
		slider_handle_html = "";
		li_html = "";

		slider_right_html = '<div class="mui-slider-right mui-disabled"><a class="mui-btn mui-btn-red mui-icon member_list icon-delete" onclick="delete_member(this)"></a></div>';

		if(role == 2) {
			icon_html = '<div class="member_icon_role2"><span class="mui-icon member_icon_font icon-shouquan2"></span></div>';
		} else if(role == 3) {
			icon_html = '<div class="member_icon_role3"><span class="mui-icon member_icon_font icon-shouquan2"></span></div>';
		} else {
			alert("云端数据错误,请退出重试");
		}

		if(role == 2) {
			if(phone != '') {
				text_html = '<div class="member_list_text"><div class="member_list_member_text">(管理员)' + phone + '</div></div>';
			} else if(phone == '') {
				text_html = '<div class="member_list_text"><div class="member_list_member_text">(管理员)' + email + '</div></div>';
			}
		} else if(role == 3) {
			if(phone != '') {
				text_html = '<div class="member_list_text"><div class="member_list_member_text">(普通用户)' + phone + '</div></div>';
			} else if(phone == '') {
				text_html = '<div class="member_list_text"><div class="member_list_member_text">(普通用户)' + email + '</div></div>';
			}
		} else {
			alert("云端数据错误,请退出重试");
		}

		slider_handle_html = '<div class="mui-slider-handle">' + icon_html + text_html + '</div>';

		li_html = '<li id="' + enduserid + '" class="mui-table-view-cell">' + slider_right_html + slider_handle_html + '</li>';
		li_total += li_html; 
	}

	if(li_total == ''){
		document.getElementById('member_list').innerHTML = '';
		document.getElementById('member_list').style.display = 'none';
		document.getElementById('share_qr_img_txt').innerHTML = '授权列表为空';
		
	}else{
		total_html = '<ul class="mui-table-view">' + li_total + '</ul>';
		document.getElementById('member_list').innerHTML = total_html;
		document.getElementById('member_list').style.display = 'block';
		document.getElementById('share_qr_img_txt').innerHTML = '左滑可删除已授权的用户';
	}
}

//删除已授权的指定用户
function delete_member(member_obj) {
	var enduserid = member_obj.parentNode.parentNode.id;
	
	var app_token = check_app_token();

	//检测网络
	if(monitor_net() == "none") {
		return;
	}

	var param = {
		deviceid: member_device_id,
		enduserid: enduserid,
		token: app_token
	};

	//alert(JSON.stringify(param));

	mico2.removeBindRole(param, function(ret, err) {
		try {
			if(ret && ret.meta.code == 0) {
				//alert(JSON.stringify(ret));
				get_member_list(member_device_id);
			} else {
				mui.toast("删除失败,请重试.err:" + JSON.stringify(err));
			}
		} catch(e) {
			mui.toast("网络不稳定,删除失败,请重试. " + e.message);
		}
	});
}


function device_management_page_listen_keyback(){
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		quit_management_page();
	});
}
