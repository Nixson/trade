var 	http 					= require('http'),
		fs 						= require('fs'),
		qs						= require('querystring'),
		url						= require('url'),
		WebSocketServer 		= require('ws').Server,
		rp 						= require('request-promise');


module.exports = function(gData){
	var info = {
		gData: gData,
		self: null,
		tick: function(){
			var self = this;
			rp(self.options)
				.then(function (repos) {
					//var insert = [];
					for(var name in repos){
						var list = repos[name];
						for(var num in list){
							if(typeof self.gData.trades[list[num].timestamp]=="undefined"){
								self.gData.trades[list[num].timestamp] = {};
							}
							if(typeof self.gData.trades[list[num].timestamp][name]=="undefined"){
								self.gData.trades[list[num].timestamp][name] = {};
							}
							if(typeof self.gData.trades[list[num].timestamp][name][list[num].tid]=="undefined"){
								self.gData.trades[list[num].timestamp][name][list[num].tid] = {
									"type": list[num].type,
									"price": list[num].price,
									"amount": list[num].amount,
								};
							}
							/*var str = '('+list[num].timestamp+',"'+name+'",'+list[num].tid + ',"'+list[num].type+'","'+list[num].price.toString()+'","'+list[num].amount.toString()+'")';
							insert.push(str);*/
						}
					}
					/*if(insert.length > 0){
						self.gData.mysql.getConnection(function(err,connection){
							connection.query('INSERT IGNORE INTO `trade_api`.`trades` (`utime`, `type`, `tid`, `st`, `price`, `amount`) VALUES '+insert.join(","),function(err,rel){
								connection.release();
							});
						});
						self.close();
					}*/
					self.close();
				})
				.catch(function (err) {
					self.gData.log(err);
			});
		},
		bind: function(){
			var self = this;
			fs.readFile(__dirname+"/trades.json",(err,data)=>{
				if(err) self.gData.log("no read file");
				else {
					self.gData.trades = JSON.parse(data);
				}
			});
			self.options = {
				uri: self.gData.config.url.trades,
				headers: {
					'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.75 Safari/537.36'
				},
				json: true
			};
		},
		close: function(){
			var self = this;
			fs.writeFile(__dirname+"/trades.json",JSON.stringify(self.gData.trades));
		}
	};
	info.self = info;
	return info;
}