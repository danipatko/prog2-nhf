import fs from "fs";
import readline from "readline";

(async () => {
    const stream = fs.createReadStream("./budapest_hungary.filtered.geojsonl");
    const write = fs.createWriteStream("./budapest_hungary.roads2.geojsonl");

    const rl = readline.createInterface({
        input: stream,
    });

    for await (const line of rl) {
        if (/"type":"LineString"|"type":"MultiPolygon"/.test(line)) {
            // console.log(line);
            write.write(line + "\n");
        }
    }

    stream.close();
    write.close();
})();
