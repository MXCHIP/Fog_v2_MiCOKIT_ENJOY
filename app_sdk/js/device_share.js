
//退出当前页面
function quit_device_share_page() {
	send_event('refesh_device_list', 'device_share');
	closeWin();
}

//当前页面监听Android的系统返回按键
function device_share_page_listen_keyback() {
	api.addEventListener({
		name: 'keyback'
	}, function(ret, err) {
		quit_device_share_page();
	});
}
