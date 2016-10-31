/*
HTML5 canvas colorpicker

Copyright (C) 2014 J.J. van Oorschot

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/*
html5, canvas, javascript colorpicker, touch friendly (no jquery)
choose a color from hsv values, used for a rgb ledstrip
uses the basis of html5 canvas timepicker
*/

//draw function to do all the drawing.
//色调H,用角度度量，取值范围为0°～360°，从红色开始按逆时针方向计算，红色为0°，绿色为120°,蓝色为240°。它们的补色是：黄色为60°，青色为180°,品红为300°；
//饱和度S,取值范围为0.0～1.0，值越大，颜色越饱和。
//亮度V,取值范围为0(黑色)～255(白色)。
colorPicker.prototype.draw = function(drawHandles) {
	if (this.changed) {
		this.ctx.save();
		//clearRect(x, y, width, height)删除一个画布的矩形区域
		//xy，左上角的坐标，wid，hei矩形的尺寸
		this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
		//beginPath() 丢弃任何当前定义的路径并且开始一条新的路径。它把当前的点设置为 (0,0)
		this.ctx.beginPath();
		//		rect() 方法为当前路径添加一条矩形子路径
		this.ctx.rect(0, 0, 2 * this.centerX, 2 * this.centerY);
		//		closePath() 方法关闭一条打开的子路径
		this.ctx.closePath();
		//		用来填充路径的当前的颜色、模式或渐变
		this.ctx.fillStyle = this.bgcolor;
		//background color
		//		fill() 方法使用 fillStyle 属性所指定的颜色、渐变和模式来填充当前路径
		this.ctx.fill();

		//draw around center
		//		translate() 方法为画布的变换矩阵添加水平的和垂直的偏移。参数 dx 和 dy 添加给后续定义路径中的所有点
		this.ctx.translate(this.centerX, this.centerY);

		//background for value ring
		//get color for display in css, hsv to hsl. value is constant 100%,
		hsl = hsv2hsl(this.h / Math.PI * 180, this.s * 100, 100);
		hsl = 'hsl(' + hsl.h + ',' + hsl.s + '%,' + hsl.l + '%)';

		//draw the arc as very thick lines
		//		指定了画笔（绘制线条）操作的线条宽度
		this.ctx.lineWidth = this.scale;

		//draw outer circle for value ring
		this.ctx.beginPath();
		//		strokeStyle 属性设置或返回用于笔触的颜色、渐变或模式
		this.ctx.strokeStyle = hsl;

		//arc(x, y, radius, startAngle, endAngle, counterclockwise)
		//x, y	描述弧的圆形的圆心的坐标。
		//radius	描述弧的圆形的半径。
		//startAngle 沿着圆指定弧的开始点和结束点的一个角度。这个角度用弧度来衡量。
		//endAngle 沿着 X 轴正半轴的三点钟方向的角度为 0，角度沿着逆时针方向而增加。
		//counterclockwise	弧沿着圆周的逆时针方向（TRUE）还是顺时针方向（FALSE）遍历。
		this.ctx.arc(0, 0, this.scale * 4 + this.scale / 2, 0, 2 * Math.PI, false);
		//		stroke() 方法绘制当前路径的边框
		this.ctx.stroke();
		//		closePath() 方法关闭一条打开的子路径
		this.ctx.closePath();

		//draw colorgradient
		//		drawImage(image, sourceX, sourceY, sourceWidth, sourceHeight,destX, destY, destWidth, destHeight)
		//image	所要绘制的图像。这必须是表示 <img> 标记或者屏幕外图像的 Image 对象，或者是 Canvas 元素。
		//x, y	要绘制的图像的左上角的位置。
		//width, height	图像所应该绘制的尺寸。指定这些参数使得图像可以缩放。
		//sourceX, sourceY	图像将要被绘制的区域的左上角。这些整数参数用图像像素来度量。
		//sourceWidth, sourceHeight	图像所要绘制区域的大小，用图像像素表示。
		//destX, destY	所要绘制的图像区域的左上角的画布坐标。
		//destWidth, destHeight	图像区域所要绘制的画布大小。
		this.ctx.drawImage(this.clrImg, -this.scale * 5, -this.scale * 5, this.scale * 10, this.scale * 10);

		//get color for center
		hsl = hsv2hsl(this.h / Math.PI * 180, this.s * 100, this.v * 100);
		hsl = 'hsl(' + hsl.h + ',' + hsl.s + '%,' + hsl.l + '%)';

		//draw inside, chosen color
		//画内圈
		//		beginPath() 丢弃任何当前定义的路径并且开始一条新的路径。它把当前的点设置为 (0,0)
		this.ctx.beginPath();
		//		arc() 方法使用一个中心点和半径，为一个画布的当前子路径添加一条弧
		//第三个值是半径
		this.ctx.arc(0, 0, this.scale * 3, 0, 2 * Math.PI, false);
		this.ctx.closePath();
		//设置内圈的颜色
		this.ctx.fillStyle = '#ffffff';
		//		this.ctx.fillStyle = hsl;
		this.ctx.fill();

		//draw the handles
		this.ctx.save();
		//color handle
		//		rotate() 方法旋转画布的坐标系统
		this.ctx.rotate(this.h);
		//操作环的位置
		//		translate(dx, dy) 方法转换画布的用户坐标系统，转换的量的 X 和 Y 大小
		this.ctx.translate(0.21 * document.body.clientHeight, 0);
		//				this.ctx.translate(100, 0);
		//		this.ctx.translate(this.scale + this.s * this.scale * 3, 0);
		//go to handle location
		this.drawHandle('c');
		//set canvas origin
		this.ctx.restore();
		//restore the canvas so that origin is in center of image
		//value handle
		//		this.ctx.rotate(this.v * 2 * Math.PI);
		//		this.ctx.translate(this.scale * 4.5, 0);
		//go to handle location
		//画外面亮度的圈
		//		this.drawHandle('v');
		this.ctx.restore();
	}
	this.changed = false;
}
//draw the circles that are used for input
colorPicker.prototype.drawHandle = function(handle) {
	var lw = Math.round(this.scale / 6.66667, 0);
	//draw handle
	this.ctx.lineWidth = lw;
	this.ctx.beginPath();
	//this.ctx.strokeStyle='white'
	this.ctx.arc(0, 0, this.scale / 2 - lw / 2, 0, 2 * Math.PI, false);
	this.ctx.closePath();

	this.ctx.fillStyle = 'rgba(0,0,0,0)';
	//color inside handle
	this.ctx.strokeStyle = (this.selected === handle) ? 'rgba(220,220,220,0.7)' : 'rgba(255,255,255,1)';
	//color of ring, first: when selected, second: other
	//	绘制一条路径
	this.ctx.stroke();
	//	fill() 方法填充当前的图像（路径）
	this.ctx.fill();
}
//return the position of the mouse relative to the canvas
colorPicker.prototype.getMousePos = function(evt) {
	var rect = this.canvas.getBoundingClientRect();
	return {
		x : evt.touches[0].pageX - rect.left,
		y : evt.touches[0].pageY - rect.top
	};
}
//returns the centers of the handles for the minutes and hours.
//Used to draw the handles and to check whether they are selected
colorPicker.prototype.getHandlers = function() {
	/*
	 v: Value, ring
	 c: Color, inner circle
	 */
	return {
		xv : this.centerX + Math.cos(this.v * 2 * Math.PI) * (this.scale * 4.5),
		yv : this.centerY + Math.sin(this.v * 2 * Math.PI) * (this.scale * 4.5),
		xc : this.centerX + Math.cos(this.h) * (this.scale * 3 * this.s + this.scale),
		yc : this.centerY + Math.sin(this.h) * (this.scale * 3 * this.s + this.scale)
	};
}
//check whether a given position (of the mouse) in xMouse and yMouse falls inside one of the handlers.
//used for checked whether handle is selected
colorPicker.prototype.contains = function(xMouse, yMouse) {
	handlersPos = this.getHandlers();
	if ((Math.pow(handlersPos.xv - xMouse, 2) + Math.pow(handlersPos.yv - yMouse, 2)) <= this.scale / 2 * this.scale / 2) {
		return 'v';
	}
	if ((Math.pow(handlersPos.xc - xMouse, 2) + Math.pow(handlersPos.yc - yMouse, 2)) <= this.scale / 2 * this.scale / 2) {
		return 'c';
	} else {
		return false;
	}
}
//convert hsv to hsl
function hsv2hsl(hue, sat, val) {
	// determine the lightness in the range [0,100]
	var l = (2 - sat / 100) * val / 2;

	// store the HSL components
	hsl = {
		'h' : hue,
		's' : sat * val / (l < 50 ? l * 2 : 200 - l * 2),
		'l' : l
	};
	// correct a division-by-zero error
	if (isNaN(hsl.s))
		hsl.s = 0;
	return hsl;
}

//set the width of the canvas
colorPicker.prototype.setWidth = function(w, h, centerX, centerY, scale) {
	this.canvas.width = w;
	this.canvas.heigth = h;
	this.ctx.canvas.width = w;
	this.ctx.canvas.height = h;
	this.centerX = centerX || w / 2;
	this.centerY = centerY || h / 2;
	this.scale = scale || Math.min(w, h) / 10;
	//size of one arc. 40 (=40px) is the same as alarms app one alarm view.
	this.changed = true;
}
//return the chosen color as HSV with values 0-1
colorPicker.prototype.getColorHSV = function() {
	return {
		h : this.h / (2 * Math.PI),
		s : this.s,
		v : this.v
	}
}
//return the chosen color as HSL with values 0-1
colorPicker.prototype.getColorHSL = function() {
	hsl = hsv2hsl(this.h, this.s, this.v)
	return {
		h : hsl.h / (2 * Math.PI),
		s : hsl.s,
		l : hsl.l
	}
}
//set the colorPicker to a given color in hsv values
colorPicker.prototype.setColorHSV = function(h, s, v) {
	this.h = h <= 1 ? h * (2 * Math.PI) : 0;
	this.s = (s <= 1 ? s : 0);
	this.v = (v <= 1 ? v : 0);
	this.changed = true;
}
//start the timer for drawing
colorPicker.prototype.startDraw = function() {
	if (!this.drawID) {
		var colorPicker = this;
		this.drawID = setInterval(function() {
			colorPicker.draw();
		}, colorPicker.drawInterval);
	}
}
//stop the timer for drawing, to save cpu
colorPicker.prototype.stopDraw = function() {
	if (this.drawID) {
		clearInterval(this.drawID);
		this.drawID = false;
	}
}

//function object for each canvas, for each colorPicker
//contains all variables for a colorPicker like scale and color.
function colorPicker(canvas, opts) {
	//init
	this.canvas = canvas;
	this.ctx = canvas.getContext('2d');
	//get the drawable part of the canvas
	var colorPicker = this;
	//image for the gradient in the center
	this.clrImg = new Image();
	this.clrImg.src = opts.image || '../img/RGB_color.jpg';
	//store (this) class in variable, so events can use (this) as well
	//default values, all zero
	this.h = 0;
	//0-2pi
	this.s = 0;
	//0-1
	this.v = 0;
	//0-1
	this.changed = true;
	this.bgcolor = opts.bgcolor || 'rgb(200,200,200)';
	//options
	this.setWidth(canvas.width, canvas.height, opts.centerX || false, opts.centerY || false, opts.scale || false);
	//this.drawHandles = (typeof opts.drawHandles === 'undefined' || opts.drawHandles == true)?true:false; //draw handles on the colorPicker. If false, it is just a clock

	//	this.animationStep = opts.animationStep || 5; 	//number of steps in handle animation
	this.drawInterval = opts.drawInterval || 30;
	//time between drawing the canvas in ms
	this.onColorChange = opts.onColorChange || false;
	//callback function
	this.onCenterClick = opts.onCenterClick || false;
	this.onFinishClick = opts.onFinishClick || false;//zfw add
	this.setColorHSV(opts.h, opts.s, opts.v);

	//start the drawing
	if (( typeof opts.autoStartDraw === 'undefined') || opts.autoStartDraw == true) {
		this.drawID = setInterval(function() {
			colorPicker.draw();
		}, colorPicker.drawInterval);
	} else {
		this.drawID = false;
	}

	//if the mouse is down, check whether it is on any of the handles
	canvas.addEventListener('touchstart', function(e) {
		//console.log(e);
		if (document.body.contains(colorPicker.canvas)) {//only if canvas is visible
			//返回鼠标相对于画布的位置
			var mouse = colorPicker.getMousePos(e);
			//检查是否一个给定的位置(鼠标)在xmouse和ymouse瀑布里面的一员。
			colorPicker.selected = colorPicker.contains(mouse.x, mouse.y);
			//this functions sets colorPicker.selected

			//colorPicker.selected始终是false
			if (!colorPicker.selected) {//if not clicked on a ring, move the ring to a position\
				var mx = mouse.x - colorPicker.centerX;
				var my = mouse.y - colorPicker.centerY;
				var len = Math.pow(mx, 2) + Math.pow(my, 2);
				//				alert(len);
				//				alert(colorPicker.scale * colorPicker.scale * 25);
				//				alert(colorPicker.scale * colorPicker.scale);
				//				alert(colorPicker.scale * colorPicker.scale * 16);
				if (len <= colorPicker.scale * colorPicker.scale * 25) {//inside all 5 rings
					if (len > colorPicker.scale * colorPicker.scale) {//outside inner color place
						var angle = Math.atan2(my, mx);
						angle += (my) < 0 ? 2 * Math.PI : 0;
						//从这里开始改，导致触摸圆的区域变大-----------------------
						colorPicker.h = angle;
						var s = (Math.sqrt(len) - colorPicker.scale) / 3 / colorPicker.scale;
						s = s > 1 ? 1 : s;
						colorPicker.s = s < 0 ? 0 : s;
						colorPicker.selected = 'c';
						if (colorPicker.onColorChange) {
							colorPicker.onColorChange();
						} //function executed when the handles are changed
						//						if (len <= colorPicker.scale * colorPicker.scale * 16) {//inside color area
						//							alert("inside color area");
						//							colorPicker.h = angle;
						//							var s = (Math.sqrt(len) - colorPicker.scale) / 3 / colorPicker.scale;
						//							s = s > 1 ? 1 : s;
						//							colorPicker.s = s < 0 ? 0 : s;
						//							colorPicker.selected = 'c';
						//							if (colorPicker.onColorChange) {
						//								colorPicker.onColorChange();
						//							} //function executed when the handles are changed
						//						} else {//inside value ring
						//							alert("inside value ring");
						//							colorPicker.v = (angle / (2 * Math.PI)) % 1;
						//							colorPicker.selected = 'v';
						//							if (colorPicker.onColorChange) {
						//								colorPicker.onColorChange();
						//							} //function executed when the handles are changed
						//						}
					}
				}
			}
			colorPicker.changed = true;
			//redraw to show selected handle on click, not only on draw
		}
	});
	//if the mouse if moved AND a handler is selected, move the handler and calculate the new time
	canvas.addEventListener('touchmove', function(e) {
		if (colorPicker.selected && document.body.contains(colorPicker.canvas)) {
			if (colorPicker.onColorChange) {
				colorPicker.onColorChange();
			}//function executed when the handles are changed
			//get mouse positions for moving
			var mouse = colorPicker.getMousePos(e);
			var mx = mouse.x - colorPicker.centerX;
			var my = mouse.y - colorPicker.centerY;
			//calculate the rotate angle from the mouse X and Y
			var angle = Math.atan2(my, mx);
			angle += my < 0 ? 2 * Math.PI : 0;
			if (colorPicker.selected == 'c') {
				colorPicker.h = angle;
				//console.log("a:"+angle);
				var s = (Math.sqrt(Math.pow(mx, 2) + Math.pow(my, 2)) - colorPicker.scale) / 3 / colorPicker.scale;
				s = s > 1 ? 1 : s;
				s = s < 0 ? 0 : s;
				colorPicker.s = s;
			} else if (colorPicker.selected == 'v') {
				colorPicker.v = (angle / (2 * Math.PI)) % 1;
			}
			//the canvas changed, if this is true, it will be redrawn
			colorPicker.changed = true;
		}
	});
	//stop the selection when the mouse is released.
	//bind this one to window to also stop the selection if mouse is released outside the canvas area.
	window.addEventListener('touchend', function(e) {
		if (colorPicker.selected && document.body.contains(colorPicker.canvas)) {
			//colorPicker.animate(colorPicker.selected,true); //animate handlers to position
			colorPicker.selected = false;
			// which handle is moving is now stored, so handle can be deselected
			colorPicker.changed = true;
		}
		if (colorPicker.onCenterClick && document.body.contains(colorPicker.canvas)) {
			var mouse = colorPicker.getMousePos(e);
			if ((Math.pow(colorPicker.centerX - mouse.x, 2) + Math.pow(colorPicker.centerY - mouse.y, 2)) < colorPicker.scale * colorPicker.scale) {
				colorPicker.onCenterClick();
			}
		}
		//zfw add
		if (colorPicker.changed && colorPicker.onFinishClick && document.body.contains(colorPicker.canvas)) {
			colorPicker.onFinishClick();
		}
	});
}

//document.body.addEventListener('touchmove', function(event) {
//	event.preventDefault();
//}, false);
