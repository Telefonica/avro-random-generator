package io.confluent.avro.random.generator.geojson;

import java.util.Random;

public class ReferenceSystemAndProjection {
    ReferenceSystem referenceSystem;
    Projection projection;

    // Minimum and maximum longitudes and latitudes for both ETRS89 and WGS84 with UTM projection are calculated for
    // UTM zones 28 to 38 (Europe and african islands owned by Europe)
    private final int minETRS89UTMLatitude = 0;
    private final int maxETRS89UTMLatitude = 10810000;
    private final int minETRS89UTMLongitude = 166000;
    private final int maxETRS89UTMLongitude = 833000;

    public ReferenceSystemAndProjection(
            ReferenceSystem referenceSystem,
            Projection projection
    ) {
        this.referenceSystem = referenceSystem;
        this.projection = projection;
    }

    public int randomLatitude() {
        Random r = new Random();

        switch(referenceSystem) {
            case ETRS89:
                switch (projection) {
                    case UTM:
                        return r.nextInt(maxETRS89UTMLatitude - minETRS89UTMLatitude) + minETRS89UTMLatitude;
                    default:
                        return 0;
                }
            case WGS84:
                switch (projection) {
                    case UTM:
                        return 1;
                    default:
                        return 0;
                }
            default:
                return 0;
        }
    }

    public int randomLongitude() {
        Random r = new Random();

        switch(referenceSystem) {
            case ETRS89:
                switch (projection) {
                    case UTM:
                        return r.nextInt(maxETRS89UTMLongitude - minETRS89UTMLongitude) + minETRS89UTMLongitude;
                    default:
                        return 0;
                }
            case WGS84:
                switch (projection) {
                    case UTM:
                        return 1;
                    default:
                        return 0;
                }
            default:
                return 0;
        }
    }
}
