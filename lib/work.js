var 	http 					= require('http'),
		fs 						= require('fs'),
		qs						= require('querystring'),
		url						= require('url'),
		WebSocketServer 		= require('ws').Server,
		staticSrv				= require('node-static'),
		fileServer				= new staticSrv.Server(__dirname+'/public',{cache: 7200}),
		Canvas					= require('canvas'),
		Image					= Canvas.Image;

module.exports = function(gData){
	var info = {
		gData: gData,
		self: null,
		cs: null,
		tick: function(){},
		bind: function(){
			var self = this;
			var _this = this;
			_this.cs = http.Server(function(request, response) {
				request.setEncoding("utf8");
				var info = url.parse(request.url,true);
				info.headers = request.headers;
				if (info.pathname=='/info'){
					self.get(response,function(){
						response.end();
					});
				}
				else if (info.pathname=='/'){
					fileServer.serveFile('/index.html', 200, {}, request, response);
				}
				else {
					request.addListener('end', function () {
						fileServer.serve(request, response, function (e, res) {
				            if (e && (e.status === 404)) { // If the file wasn't found
				                fileServer.serveFile('/error/not-found.html', 404, {}, request, response);
				            }
				        });
					}).resume();
				}
			});
			_this.cs.timeout = 0;
			_this.cs.listen(_this.gData.config.srv.worker);
		},
		clean: function(){

		},
		close: function(){
			this.cs.close();
		},
		startScan: function(){
			var canvas = new Canvas(4300, 1024)
			var ctx = canvas.getContext('2d');
			ctx.lineWidth = 1;
			ctx.fillStyle = "rgba(255,255,255,1.0)";
//			ctx.globalAlpha = .9;
			//ctx.strokeStyle = 'red';
			var _this = this;
			var self = this;
			var time = _this.gData.time()-86400;
			var blockList = {};
			var maxArg = 0;
			var minArg = 0;
			var maxColorAsk = 0;
			var maxColorBid = 0;
			for( var name in self.gData.depths){
				for( var type in self.gData.depths[name]){
					for( var dtime in self.gData.depths[name][type]){
						if(maxArg < self.gData.depths[name][type][dtime].max)
							maxArg = self.gData.depths[name][type][dtime].max;
						if(minArg > self.gData.depths[name][type][dtime].min || minArg == 0)
							minArg = self.gData.depths[name][type][dtime].min;
						if(type=="asks"){
							if(maxColorAsk < self.gData.depths[name][type][dtime].maxColor)
								maxColorAsk = self.gData.depths[name][type][dtime].maxColor;
						}else {
							if(maxColorBid < self.gData.depths[name][type][dtime].maxColor)
								maxColorBid = self.gData.depths[name][type][dtime].maxColor;
						}
					}
				}
			}

			for( var name in self.gData.depths){
				console.log(name);
				for( var type in self.gData.depths[name]){
					//сначала рисуем asks
					console.log(type);
					var color = 'rgba(0,0,0,';
					var maxColor = maxColorAsk;
					switch(type){
						case "asks":
									color = 'rgba(250,0,0,';
									maxColor = maxColorAsk;
									break;
						case "bids":
									color = 'rgba(0,0,250,';
									maxColor = maxColorBid;
									break;
					}
					var tPos = 0;
					for(var st = time; st <= _this.gData.time(); st+=60){
						var sblock = {};
						for(var subs = st; subs < st+60; subs++){
							if(typeof self.gData.depths[name][type][subs]!='undefined'){
								sblock[subs] = self.gData.depths[name][type][subs].list;
							}
						}
						//console.log(st);
						var scan = self.scan(minArg,maxArg,sblock);
						self.lineTo(ctx,maxColor,tPos,color,scan);
						tPos+=3;
//						blockList[st] = scan;
						delete sblock;
					}
				}
			}
			ctx.stroke();
			 ctx.fill();
			canvas.toBuffer(function(err, buf){
				fs.writeFile(__dirname + '/sparks.png', buf);
				console.log("done");
			});
			console.log("stat");
			//console.log(blockList);
		},
		lineTo: function(ctx,maxColor,pos,color,list){
			var newmc = maxColor;
			for (var nm in list){
				if(newmc < list[nm][1]){
					newmc = list[nm][1];
				}
			}
			for( var num in list){
				var colorSt = (list[num][1]/newmc).toFixed(3);
				var style = color+colorSt.toString()+')';
				ctx.strokeStyle = style;
				ctx.fillStyle = style;
				var y = parseInt(num)+20;
				ctx.beginPath();
				ctx.moveTo(pos,y);
				ctx.lineTo(pos+3,y);
				ctx.stroke();
			}
		},
		scan: function(min, max, block){	//по 1-й минуте.
			var norm = (max - min)/984;
			return this.normW(this.normH(min, norm,block));
		},
		normH: function(min,norm, list){
			var summ = 0;
			var result = {};
			for(var sec in list){
				var mBlock = {};
				for( var s = 0; s < 984; s++)
					mBlock[s] = {summA:0.0, summC:0.0,cnt:0};
				mBlock[984] = 0;
				for(var nm in list[sec] ){
					var arg = parseInt((list[sec][nm][0]-min)/norm);
					mBlock[arg].summA+=list[sec][nm][0];
					mBlock[arg].summC+=list[sec][nm][1];
					mBlock[arg].cnt++;
				}
				result[sec] = [];
				for( var s = 0; s < 984; s++){
					if(mBlock[s].cnt > 0)
						result[sec].push( [ mBlock[s].summA/mBlock[s].cnt, mBlock[s].summC/mBlock[s].cnt ]);
					else
						result[sec].push( [ 0, 0 ]);
				}
				delete mBlock;
			}
			return result;
		},
		normW: function(list){
			var result = [];
			var cnt = 0;
			for(var sec in list){
				cnt++;
			}
			if(cnt==0) cnt = 1;
			for( var s = 0; s < 984; s++){
				var v0 = 0.0;
				var v1 = 0.0;
				for(var sec in list){
					v0+=list[sec][s][0];
					v1+=list[sec][s][0];
				}
				result.push([v0/cnt,v1/cnt]);
			}
			delete list;
			return result;
		},
		image: function(data){
			var canvas = new Canvas(4300, 1024)
			var ctx = canvas.getContext('2d');


		},
		get: function(response,callback){
			var self = this;
			var _this = this;
			/*process.nextTick(function(){
				self.startScan();
			});*/
			
			response.writeHead(200, {"Content-Type": "application/json; charset=utf8"});
			response.write("{");
			var maxArg = 0;
			var minArg = 0;
			for( var name in self.gData.depths){
				for( var type in self.gData.depths[name]){
					for( var dtime in self.gData.depths[name][type]){
						if(maxArg < self.gData.depths[name][type][dtime].max)
							maxArg = self.gData.depths[name][type][dtime].max;
						if(minArg > self.gData.depths[name][type][dtime].min || minArg == 0)
							minArg = self.gData.depths[name][type][dtime].min;
					}
				}
			}
			response.write('"max":'+maxArg+',"min":'+minArg+',"depths":{');
			var minTdime = "";
			var fName = true;
			/*for( var name in self.gData.depths){
				if(!fName)
						response.write(',');
					else
						fName = false;
				response.write('"'+name+'":{');
				var fType = true;
				for( var type in self.gData.depths[name]){
					if(!fType)
							response.write(',');
						else
							fType = false;
					response.write('"'+type+'":{');
					var isFirts = true;
					for( var dtime in self.gData.depths[name][type]){
						if(!isFirts)
							response.write(',');
						else{
							minTdime = dtime;
							isFirts = false;
						}
						response.write('"'+dtime+'":');
						var len = 100/self.gData.depths[name][type][dtime].list.length;
						var pres = [];
						var maxColor = 0;
						var minColor = 0;
						for(var num in self.gData.depths[name][type][dtime].list){
//							var color =parseInt((self.gData.depths[name][type][dtime][num][1]-minColor)*(maxColor-minColor)/255);
							var pos = parseInt(len*num);
							if(typeof pres[pos]=='undefined')
								pres.push([0,0]);
							pres[pos][0]+=self.gData.depths[name][type][dtime].list[num][0];
							pres[pos][1]+=self.gData.depths[name][type][dtime].list[num][1];
//							response.write('['+self.gData.depths[name][type][dtime][num][0]+','+color+']');
						}
						for( var pos in pres){
							if(maxColor < pres[pos][1])
								maxColor = pres[pos][1];
							if(minColor > pres[pos][1] || minColor==0)
								minColor = pres[pos][1];
						}
						for( var pos in pres){
							var resp = parseInt(255*(pres[pos][1]-minColor)/(maxColor-minColor));
							pres[pos][1] = resp;
						}
						response.write(JSON.stringify(pres));
						//response.write(']');
					}
					response.write('}');
				}
				response.write('}');
			}*/
			response.write('},"trades":[');
			minTdime = parseInt(minTdime);
			var isFirts = true;
			for( var dtime in self.gData.trades){
				//console.log(dtime);
				var tDstr = parseInt(dtime);
				if(tDstr < minTdime)
					continue;
				if (!isFirts)
					response.write(',');
				else
					isFirts = false;
				response.write('{"'+dtime.toString()+'":'+JSON.stringify(self.gData.trades[dtime])+'}');
			}
			response.write(']}\n');
			callback();
		}
	};
	info.self = info;
	return info;
}