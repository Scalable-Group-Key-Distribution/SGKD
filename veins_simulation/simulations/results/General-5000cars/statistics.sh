for metric in registrationLatency registrationProcessingLatency updateLatency updateProcessingLatency; do
    echo "=== $metric ==="

    opp_scavetool query -T s -l \
        -f "name =~ \"*$metric*\"" *.sca |
    awk '
    $NF ~ /^[0-9.eE+-]+$/ {
        x = $NF * 1000  # Convert seconds to milliseconds
        values[++n] = x
        sum += x

        if (n == 1 || x < min) min = x
        if (n == 1 || x > max) max = x
    }
    END {
        if (n == 0) {
            print "No values found"
            exit
        }

        mean = sum / n

        # Sample standard deviation
        for (i = 1; i <= n; i++) {
            diff = values[i] - mean
            ss += diff * diff
        }

        if (n > 1) {
            sd = sqrt(ss / (n - 1))
            se = sd / sqrt(n)

            # Approximate 95% CI using 1.96 (normal approximation)
            margin = 1.96 * se
            lower = mean - margin
            upper = mean + margin
        } else {
            sd = 0
            se = 0
            lower = mean
            upper = mean
        }

        printf "Count:                  %d\n", n
        printf "Sum:                    %.6f ms\n", sum
        printf "Average:                %.6f ms\n", mean
        printf "Minimum:                %.6f ms\n", min
        printf "Maximum:                %.6f ms\n", max
        if (n > 1) {
            printf "Sample std deviation:   %.6f ms\n", sd
            printf "Standard error:         %.6f ms\n", se
            printf "95%% confidence interval: [%.6f, %.6f] ms\n", lower, upper
        } else {
            print "Standard deviation and confidence interval require more observations."
        }
    }'

    echo
done
