// Copyright 2018 Ulf Adams
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package info.adams.ryu.benchmark;

final class MeanAndVariance {
  private long n;
  private double mean;
  private double m2;

  public MeanAndVariance() {
  }

  public void update(double x) {
    n++;
    double d = x - mean;
    mean += d / n;
    double d2 = x - mean;
    m2 += d * d2;
  }

  public double mean() {
    return mean;
  }

  public double variance() {
    return m2 / (n - 1);
  }

  public double stddev() {
    return Math.sqrt(variance());
  }
}
