$(document).ready(function() {
    
    // 1. Cookie Language Management
    $('.lang-btn').click(function() {
        let selectedLang = $(this).data('lang');
        
        // Save choice for 1 year across the domain
        document.cookie = "locale=" + selectedLang + "; path=/; max-age=" + (365 * 24 * 60 * 60);
        
        // Reload to let backend update layout text strings
        location.reload();
    });

    // 2. Client-Side Trail Search Filter
    $('#trailSearch').on('keyup', function() {
        let value = $(this).val().toLowerCase();
        let matches = 0;

        $('.trail-card').each(function() {
            let isVisible = $(this).data('name').indexOf(value) > -1;
            $(this).toggle(isVisible);
            if (isVisible) matches++;
        });

        // Toggle localized empty results block fallback
        if (matches === 0) {
            if ($('#emptyState').length === 0) {
                let localizedMsg = $('#searchHeader').data('no-results');
                $('#trailGrid').append(`<div class="col-12" id="emptyState"><p class="text-center text-muted">${localizedMsg}</p></div>`);
            }
        } else {
            $('#emptyState').remove();
        }
    });

     // 3. Manual Button Click Pagination
    let page = 1;

    $('#loadMoreBtn').click(function() {
        let $btn = $(this);
        let $spinner = $('#btnSpinner');
        
        // Disable actions during pending loading states
        $btn.prop('disabled', true);
        $spinner.removeClass('d-none');
        page++;

        $.ajax({
            url: '/routes/load-more?page=' + page,
            type: 'GET',
            success: function(htmlResponse) {
                // Check if the server returned empty content
                if ($.trim(htmlResponse) === '') {
                    $btn.data('end-of-data', true);
                    $('#loadMoreWrapper').addClass('d-none');
                } else {
                    // Append new cards directly into the DOM
                    $('#trailGrid').append(htmlResponse);
                    $btn.prop('disabled', false);
                }
                $spinner.addClass('d-none');
            },
            error: function() {
                $btn.prop('disabled', false);
                $spinner.addClass('d-none');
            }
        });
    });

      $('#logoutBtn').click(function(e) {
        e.preventDefault();

        // Dispatch verification command to your server-side session destroyer
        $.ajax({
            url: '/admin/logout',
            type: 'GET',
            dataType: 'json',
            success: function(response) {
                // If backend session clearance verifies true, execute fresh template load
                if (response.success) {
                    location.reload();
                }
            },
            error: function() {
                // Fallback option in case network state errors interrupt response payloads
                location.reload();
            }
        });
    });
    
    
});