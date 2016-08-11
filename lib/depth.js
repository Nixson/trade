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
		options: {},
		last: {},
		tick: function(){
			return true;
			var self = this;
			rp(self.options)
				.then(function (repos) {
					var dtime = self.gData.last.toString();
					var last = {};
					for(var name in repos){
						if(typeof self.gData.depths[name]=='undefined'){
							self.gData.depths[name] = {"asks":{},"bids":{}};
						}
						var last = {"asks":
												{"list":[],"min":0,"max":0,"minColor":0,"maxColor":0},
											"bids":
											{"list":[],"min":0,"max":0,"minColor":0,"maxColor":0}};
						var maxAskVal = 0;
						var minAskVal = 0;
						var maxAskColor = 0;
						var minAskColor = 0;
						for(var aNum in repos[name].asks){
							if(maxAskVal < repos[name].asks[aNum][0])
								maxAskVal = repos[name].asks[aNum][0];
							if(minAskVal > repos[name].asks[aNum][0]  || minAskVal==0)
								minAskVal = repos[name].asks[aNum][0];
							if(maxAskColor < repos[name].asks[aNum][1])
								maxAskColor = repos[name].asks[aNum][1];
							if(minAskColor > repos[name].asks[aNum][1]  || minAskColor==0)
								minAskColor = repos[name].asks[aNum][1];
						}
						last.asks.min = minAskVal;
						last.asks.max = maxAskVal;
						last.asks.minColor = minAskColor;
						last.asks.maxColor = maxAskColor;
						for(var aNum in repos[name].asks){
							last.asks.list.push([repos[name].asks[aNum][0],repos[name].asks[aNum][1]]);
						}
						var maxBidVal = 0;
						var minBidVal = 0;
						var maxBidColor = 0;
						var minBidColor = 0;
						for(var aNum in repos[name].bids){
							if(maxBidVal < repos[name].bids[aNum][0])
								maxBidVal = repos[name].bids[aNum][0];
							if(minBidVal > repos[name].bids[aNum][0] || minBidVal==0){
								minBidVal = repos[name].bids[aNum][0];
							}
							if(maxBidColor < repos[name].bids[aNum][1])
								maxBidColor = repos[name].bids[aNum][1];
							if(minBidColor > repos[name].bids[aNum][1] || minBidColor==0){
								minBidColor = repos[name].bids[aNum][1];
							}
						}
						last.bids.min = minBidVal;
						last.bids.max = maxBidVal;
						last.bids.minColor = minBidColor;
						last.bids.maxColor = maxBidColor;
						for(var aNum in repos[name].bids){
							//var color = (repos[name].bids[aNum][1]-minBidColor)*(maxBidColor-minBidColor)/255;
							last.bids.list.push([repos[name].bids[aNum][0],repos[name].bids[aNum][1]]);
						}
						self.gData.depths[name].asks[dtime] = last.asks;
						self.gData.depths[name].bids[dtime] = last.bids;
					}
					/*self.gData.mysql.getConnection(function(err,connection){
						var SQL = 'INSERT INTO `trade_api`.`depth` (`utime`, `type`, `asks`, `bids`) VALUES ';
						var dtime = self.gData.last;
						var SQL_list = [];
						for(var name in repos){
							if(typeof self.last=='undefined'){
								self.last = {"asks":"","bids":""};
							}
							var maxAsk = 0;
							for(var aNum in repos[name].asks){
								if(maxAsk < repos[name].asks[aNum][0])
									maxAsk = repos[name].asks[aNum][0];
							}
							var maxBid = 0;
							for(var aNum in repos[name].bids){
								if(maxBid < repos[name].bids[aNum][0])
									maxBid = repos[name].bids[aNum][0];
							}
							var normAsk = 1000/maxAsk;
							var nList = {};
							for(var aNum in repos[name].asks){
								var level = parseInt(repos[name].asks[aNum][0]*normAsk);
								if(typeof nList[level]=='undefined')
									nList[level] = 0;
								nList[level] += repos[name].asks[aNum][1];
							}
							var asks = JSON.stringify(nList);
							var nList = {};
							for(var aNum in repos[name].bids){
								var level = parseInt(repos[name].bids[aNum][0]*normAsk);
								if(typeof nList[level]=='undefined')
									nList[level] = 0;
								nList[level] += repos[name].bids[aNum][1];
							}
							var bids = JSON.stringify(nList);
							if(self.last.asks!=asks || self.last.bids!=bids){
								SQL_list.push('('+dtime+',"'+name+'","'+asks+'","'+bids+'")'); 
							}
						}
						if(SQL_list.length > 0)
							connection.query(SQL+SQL_list.join(','),function(err,rel){
								connection.release();
							});
					});*/
				})
				.catch(function (err) {
					self.gData.log(err);
			});
		},
		bind: function(){
			this.options = {
				uri: this.gData.config.url.depth,
				headers: {
					'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.75 Safari/537.36'
				},
				json: true
			};
		},
		close: function(){}
	};
	info.self = info;
	return info;
}