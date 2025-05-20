import fs from "fs";
import readline from "readline";

(async () => {
    const stream = fs.createReadStream("./budapest_hungary.roads2.geojsonl");
    const write = fs.createWriteStream("./budapest.mid.roads.geojsonl");

    const rl = readline.createInterface({
        input: stream,
    });

    const TL = [18.660278, 47.299722];
    const BR = [19.436874, 47.651975];

    for await (const line of rl) {
        const json = JSON.parse(line);

        const c = [...json["geometry"]["coordinates"]];
        let prevX = -1,
            prevY = -1;

        if (c.some(([x, y]) => TL[0] < x && x < BR[0] && TL[1] < y && y < BR[1])) {
            write.write(line + "\n");
        }

        // for (const [x, y] of c) {
        //     if (TL[0] < x && x < BR[0] && TL[1] < y && y < BR[1]) {
        //         if (prevX > 0 && prevY > 0) {
        //             write.write(line + "\n");

        //             // write.write(`V(glm::vec2(${prevX - 19}, ${prevY - 47})),\n`);
        //             // write.write(`V(glm::vec2(${x - 19}, ${y - 47})),\n`);ű
        //             // write.write(`add({${x - 19}, ${y - 47}}, {${prevX - 19}, ${prevY - 47}});\n`);
        //         }

        //         prevX = x;
        //         prevY = y;
        //     }
        // }
        // V(glm::vec2(0.190549401, 0.47476117700000003)),
        // V(glm::vec2(0.190551858, 0.474762439)),

        // if (/\"highway\":\"(trunk|motorway|primary|secondary|tetriary|unclassified|residential|living_street)(|_link)\"/.test(line)) {
        //     console.log(line);
        //     // write.write(line + "\n");
        // }
    }

    stream.close();
    write.close();
})();
