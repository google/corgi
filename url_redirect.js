// Copyright 2015 Google Inc. All rights reserved.
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

/**
 * @fileoverview Conditionally redirect to doxygen generated HTML files.
 *
 * When generating docs with doxygen, the tool creates its own HTML
 * file names for the generated output. This can be problematic, as updates
 * to the doxygen input files may result in different generated HTML file names
 * in the doxygen output.
 *
 * Since external sites cannot reliably link to pages with variable URLs, these
 * functions allow you to create a fixed HTML page that can be provided to
 * external sites, which conditionally redirects to a doxygen generated file.
 * The fixed HTML page can use these functions to check if the generated HTML
 * file name is valid, while providing a fixed, fallback URL to redirect to if
 * the doxygen generated HTML file names have changed.
 *
 * NOTE: In order to use these functions, the fixed HTML file will need to load
 * the jQuery library.
 */

/**
  * Check if a given URL is valid (status code 200) and redirect appropriately.
  *
  * Note: This function requires the jQuery library.
  *
  * @param {string} url The URL to check via HTTP GET request.
  * @param {string} fallbackUrl The URL to use if the provided 'url' is invalid.
  * @param {function(number, string, string)} callback A callback function that
  *     is executed after the Ajax request completes.
  */
function checkRedirectUrl(url, fallbackUrl, callback) {
  jQuery.ajax({
    url: url,
    /**
      * Applies the Ajax HTTP status code and URLs to the 'callback' function.
      * @param {Object} xhr The XMLHttpRequest object from the Ajax request.
      */
    complete: function(xhr) {
      callback.apply(this, [xhr.status, url, fallbackUrl]);
    }
  });
}

/**
  * Redirects to a given URL if the 'status' is 200.
  * @param {number} status An HTTP status code.
  * @param {string} url The URL to redirect to if the 'status' is 200.
  * @param {string} fallbackUrl The URL to redirect to if 'status' is not
  *     200 (indicating that 'url' is not a valid page to redirect to).
  */
function conditionalRedirect(status, url, fallbackUrl) {
  if (status === 200) {
    window.location.replace(url);
  } else {
    window.location.replace(fallbackUrl);
  }
}
