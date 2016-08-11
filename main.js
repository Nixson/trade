var fs 						= require('fs'),
	mysql					= require('mysql');
var trPath = __dirname+"/lib/trades.js",
	dePath = __dirname+"/lib/depth.js",
	woPath = __dirname+"/lib/work.js";
var		Trade				= require(trPath),
		Depth				= require(dePath),
		Worker				= require(woPath);
var logStream = openLog(__dirname+"/api.trade.log");

function log(msg) {
    logStream.write(msg + "\n");
}
function openLog(logfile) {
    return fs.createWriteStream(logfile, {
        flags: "a", encoding: "utf8", mode: 0644
    });
}


var config = JSON.parse(fs.readFileSync(__dirname+"/config.json", "utf8").toString()),
	pool  = mysql.createPool({
		connectionLimit : 20,
		host			: config.mysql.host,
		user			: config.mysql.user,
		database		: config.mysql.database,
		charset			: config.mysql.charset,
		password		: config.mysql.password
	}),
	gData = {
		mysql: pool,
		config: config,
		trade: null,
		depth: null,
		depths: {},
		lastDep: 0,
		trades: {},
		lastTrade: 0,
		work: null,
		time: function(){
			return parseInt((new Date).getTime()/1000);
		},
		log: function(msg){
			console.log(msg);
//			log(msg);
		},
		last: 0,
		ticks: ["depth","trade","work"],
		start: function(tickName){
			process.nextTick(function(){
				gData[tickName].tick();
			});
		}
	};

fs.watchFile(__dirname+"/config.json",function (current, previous) {
	if (current.mtime.toString() !== previous.mtime.toString()) {
		var config = JSON.parse(fs.readFileSync(__dirname+"/config.json", "utf8").toString());
		gData.config = config;
		console.log("reload config");
	}
});
function watchInfo(path,name){
	var Info = require(path);
	gData[name] = new Info(gData);
	gData[name].bind();
	fs.watchFile(path,function (current, previous) {
		if (current.mtime.toString() !== previous.mtime.toString()) {
			delete require.cache[require.resolve(path)];
			var Info = require(path);
			gData[name].close();
			delete gData[name];
			gData[name] = new Info(gData);
			gData[name].bind();
			console.log("reload",name);
		}
	});
}
watchInfo(trPath,"trade");
watchInfo(dePath,"depth");
watchInfo(woPath,"work");


gData.last = gData.time();
setInterval(function(){
	var dt = gData.time();
	if(dt > gData.last){
		gData.last = dt;
		for(var tickNum in gData.ticks){
			gData.start(gData.ticks[tickNum]);
		}
	}
},100);
setInterval(function(){
	gData.work.clean();
},3600000);


/*process.on("uncaughtException", function(err) {
    log(err.stack);
});
*/

process.once("SIGTERM", function() {
	for(var tickNum in gData.ticks){
		gData[gData.ticks[tickNum]].close();
		process.exit(0);
	}
});
